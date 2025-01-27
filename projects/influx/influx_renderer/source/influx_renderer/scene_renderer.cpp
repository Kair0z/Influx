#include "renderer_pch.h"
#include "scene_renderer.h"

// influx::renderer
#include "influx_renderer/target.h"
#include "influx_renderer/pipeline/pipeline.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/renderer_common.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

namespace influx::renderer
{
    static pipeline_signature k_scene_pipeline_signature
    {
        .m_bindless             { true },
        .m_vs_name              { "basepass_vs" },
        .m_ps_name              { "basepass_ps" },

        .m_primitive_type       { pipeline_signature::primitive_type::triangle },
        .m_cullmode             { pipeline_signature::cullmode::none },
        .m_fillmode             { pipeline_signature::fillmode::solid },
        .m_forced_samplecount   { 0u },
        .m_sample_mask          { (uint32)-1 },
        .m_sample_count         { 1u },
        .m_front_ccw            { false },
        .m_depthclip            { false },
        .m_multisample          { false },
        .m_antialiased_line     { false },
        .m_conservative_raster  { false },
        .m_depthbias            { 0 },
        .m_depthbias_clamp      { 0.0f },
        .m_slope_depthbias      { 0.0f },

        .m_depth_enable         { true },
        .m_stencil_enable       { false },
        .m_depth_comparison     { 0u },
        .m_depth_format         { pipeline_signature::format::default_depth },

        .m_rtv_actives          { true, false, false, false, false, false, false, false },
        .m_rtv_formats          { pipeline_signature::format::default_color, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
        .m_blend_actives        { false, false, false, false, false, false, false, false },
        .m_blend_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_writemasks     { pipeline_signature::blendmask::blend_all, 0u, 0u, 0u, 0u, 0u, 0u, 0u}
    };

    scene_renderer::scene_renderer(renderer_backend* backend, graphics::device* device, pipeline* pipeline)
        : mp_pipeline{ pipeline }
        , mp_backend{ backend }
        , mp_device{ device }
    {
        m_instance_data = new gpu_instance_data[k_max_num_instances]{};

        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;

            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_num_instances * sizeof(gpu_instance_data);
            desc.m_bytestride = sizeof(gpu_instance_data);
            desc.m_init_state = graphics::e_resource_state::gen_read;
            mp_instancebuffer = device->create_resource(desc, heap_desc);
            mp_instancebuffer->set_name({ "scene_instance_buffer" });

            // create srv
            m_instance_buffer_srv = backend->get_descriptor_manager()->create_buffer_srv(mp_instancebuffer);
        }

        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;

            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_num_vertices * sizeof(vertex_data);
            desc.m_bytestride = sizeof(vertex_data);
            desc.m_init_state = graphics::e_resource_state::gen_read;
            m_vertexbuffer.m_resource = device->create_resource(desc, heap_desc);
            m_vertexbuffer.m_resource->set_name({ "scene_vertexbuffer" });

            // create srv
            m_vertexbuffer.m_vertex_buffer_srv = backend->get_descriptor_manager()->create_buffer_srv(m_vertexbuffer.m_resource);
        }

        descriptor_manager& descriptor_manager = *backend->get_descriptor_manager();
        m_sampler_view = descriptor_manager.create_sampler();
    }

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

#pragma region batch
    class batch final
    {
    public:
        batch() = default;

        const material* m_material;
        vector<gpu_instance_data> m_instances;
        graphics::resource* m_indexbuffer;
        uint32 m_base_instance;
        uint32 m_base_vertex;
    };
#pragma endregion

    inline gpu_instance_data translate(const mesh_instance& mesh)
    {
        gpu_instance_data instance_data{};
        instance_data.m_transform = mesh.m_transform;
        instance_data.m_colour = mesh.m_per_instance_colour;
        return instance_data;
    }

    vector<batch> scene_renderer::create_batches(const scene& scene)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        using indexbuffer_instance_map = umap<graphics::resource*, vector<gpu_instance_data>>;
        indexbuffer_instance_map idxbuffer_to_instances{};

        const material* first_material = nullptr;
        for (const mesh_instance& instance : scene.m_meshes)
        {
            graphics::resource* indexbuffer = nullptr;
            graphics::resource* vertexbuffer = nullptr;
            if (backend.get_mesh_buffers(instance.m_name, vertexbuffer, indexbuffer))
            {
                gpu_instance_data gpu_data = translate(instance);
                idxbuffer_to_instances[indexbuffer].push_back(gpu_data);

                first_material = instance.m_material;
            }
        }

        vector<batch> batches{};
        for (const auto& per_indexbuffer : idxbuffer_to_instances)
        {
            batches.push_back({});
            batch& batch = batches.back();

            batch.m_material = first_material;
            batch.m_base_instance = 0u;
            batch.m_base_vertex = 0u;
            batch.m_indexbuffer = per_indexbuffer.first;
            batch.m_instances = per_indexbuffer.second;
        }
        return batches;
    }

    void scene_renderer::update_buffers(const vector<batch>& batches)
    {
        mp_instancebuffer->map([&batches](void* dest)
        {
            gpu_instance_data* data = reinterpret_cast<gpu_instance_data*>(dest);
            for (const batch& batch : batches)
            {
                for (size_t i = 0u; i < batch.m_instances.size(); ++i)
                {
                    data[batch.m_base_instance + i] = batch.m_instances[i];
                }
            }
        });

        m_vertexbuffer.update_buffer();
    }


    void scene_renderer::render_basepass(graphics::commandlist* commandlist, 
        const scene& scene, const vector<batch>& batches, const target& target)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        descriptor_manager& descriptor_manager = *backend.get_descriptor_manager();

        // update pipeline
        apply_pipeline_settings();
        mp_pipeline = mp_backend->get_pipeline_manager()->get_or_create_pipeline("pip_scene", k_scene_pipeline_signature);
        if (mp_pipeline == nullptr || batches.empty())
        {
            return;
        }

        logonce(e_log_category::warning, "influx::renderer::scene_renderer: first scene render!");
        
        // stage all srv/uav/const descriptors on the bindless heap
        // in bindless, MUST happen before setting descriptor (mp_pipeline->set_state)
        {
            const material& first_material = *batches[0u].m_material;
            vector<graphics::descriptor_handle> all_descriptors
            {
                backend.find_texture(first_material.get_texture_diffuse_name())->get_srv(),
                m_instance_buffer_srv,
                m_vertexbuffer.m_vertex_buffer_srv
            };

            graphics::descriptor_range gpu_range = descriptor_manager.stage(all_descriptors);
            graphics::descriptor_range gpu_range_samp = descriptor_manager.stage_sampler(m_sampler_view);
            // mp_pipeline->set_resource_table(commandlist, "all_descriptors", gpu_range);
            descriptor_manager.start_commandlist(commandlist);
        }

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        {
            mp_pipeline->set_state(commandlist);
            commandlist->set(graphics::e_primitive_topology::trilist);
        }

        // update viewprojection matrix
        {
            const camera& camera = scene.m_camera;
            math::transform3D transform = camera.m_transform;
            transform.update_matrix();

            const float ar = (float)target.get_width() / target.get_height();
            m_gpu_perview.m_vp = make_viewprojection(transform.get_matrix(), ar, camera.m_fov, camera.m_near_plane, camera.m_far_plane);
            mp_pipeline->set_constants<gpu_perscene>(commandlist, "g_perscene", m_gpu_perscene);
            mp_pipeline->set_constants<gpu_perview>(commandlist, "g_perview", m_gpu_perview);
        }

        for (const batch& batch : batches)
        {
            const material& material = *batch.m_material;

            // set constants
            m_gpu_perdraw.m_start_instance = batch.m_base_instance;
            m_gpu_perdraw.m_start_vertex = batch.m_base_vertex;
            m_gpu_permaterial.m_colour = material.get_basecolour();

            mp_pipeline->set_constants(commandlist, "g_permaterial", m_gpu_permaterial);
            mp_pipeline->set_constants(commandlist, "g_perdraw", m_gpu_perdraw);

            // bind index buffer
            graphics::resource* index_buffer = batch.m_indexbuffer;
            const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();
            commandlist->set_indexbuffer(index_buffer);

            const vector<gpu_instance_data>& instances = batch.m_instances;
            commandlist->draw_indexed(
            {
                .m_num_indexes_per_instance = num_indices,
                .m_num_instances = (uint32)instances.size(),
                .m_start_index = 0u,
                .m_start_vertex = 0,
                .m_start_instance = 0
            });
        }
    }

    void scene_renderer::apply_pipeline_settings()
    {
        const render_settings& settings = renderer_backend::get_instance().get_settings();
        
        // fillmode
        k_scene_pipeline_signature.m_fillmode = settings.m_wireframe ? pipeline_signature::fillmode::wireframe : pipeline_signature::fillmode::solid;
        
        // cullmode
        switch (settings.m_cullmode)
        {
        case render_settings::cullmode::back:  k_scene_pipeline_signature.m_cullmode = pipeline_signature::cullmode::back; break;
        case render_settings::cullmode::front: k_scene_pipeline_signature.m_cullmode = pipeline_signature::cullmode::front; break;
        case render_settings::cullmode::none:  k_scene_pipeline_signature.m_cullmode = pipeline_signature::cullmode::none; break;
        }
    }

    void scene_renderer::mega_vertexbuffer::update_buffer()
    {
        if (m_resource == nullptr || m_meshnames.empty())
        {
            return;
        }

        reset();

        uint64 total_bytesize = 0u;
        const renderer_backend& backend = renderer_backend::get_instance();
        vector<gpu_vertex_data> gpu_data{};

        // gather a mega gpu_vertexdata vector
        for (const string& name : m_meshnames)
        {
            const vector<vertex_data> vertexbuffer_content = backend.get_vertexbuffer_content<vertex_data>(name);
            if (vertexbuffer_content.size() > 0u)
            {
                const uint64 old_size = gpu_data.size();
                const uint64 num_vertices = vertexbuffer_content.size();
                const uint64 bytesize = num_vertices * sizeof(vertex_data);
                gpu_data.resize(old_size + num_vertices);

                // copy the individual vertexbuffer content into our gpu data mega-vector
                memcpy(&gpu_data[old_size], vertexbuffer_content.data(), bytesize);

                // keep the base offset
                m_meshname_to_offset[name] = old_size;

                total_bytesize += bytesize;
            }
        }

        // map onto the resource
        m_resource->map([total_bytesize, &gpu_data](void* dest)
            {
                gpu_vertex_data* data = reinterpret_cast<gpu_vertex_data*>(dest);
                memcpy(data, gpu_data.data(), total_bytesize);
            });
    }

    void scene_renderer::render(graphics::commandlist* commandlist, const scene& scene, const target& target)
    {
        m_gpu_perscene.m_time.x = scene.m_delta_seconds;
        m_gpu_perscene.m_time.y = scene.m_seconds;

        // update vertexbuffer
        for (const mesh_instance& instance : scene.m_meshes)
        {
            // register to the shared vertexbuffer
            m_vertexbuffer.register_mesh(instance.m_name);
        }
        m_vertexbuffer.update_buffer();

        // setup a batched draw
        vector<batch> batches = create_batches(scene);

        update_buffers(batches);

        // render_shadows(commandlist, scene, batches);
        render_basepass(commandlist, scene, batches, target);
    }
}