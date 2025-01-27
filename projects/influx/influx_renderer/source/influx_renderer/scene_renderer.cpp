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

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

#pragma region batch
    class batch final
    {
    public:
        batch() = default;

        material* m_material;
        uint32 m_base_instance;
        uint32 m_base_vertex;

        using mesh_to_instances_map = umap<string, vector<gpu_instance_data>>;
        
        umap<material*, mesh_to_instances_map> m_material_to_mesh_to_instances_map;

        vector<gpu_instance_data> m_instances;
        vector<gpu_vertex_data> m_vertex_buffers;
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
        using material_instance_map = umap<const material*, vector<gpu_instance_data>>;
        material_instance_map material_to_instance_map{};

        for (const mesh_instance& instance : scene.m_meshes)
        {
            gpu_instance_data gpu_data = translate(instance);
            material_to_instance_map[instance.m_material].push_back(gpu_data);
        }

        vector<batch> batches{};
        for (const auto& per_material : material_to_instance_map)
        {
            batches.push_back({});
            batch& batch = batches.back();

            batch.m_base_vertex;
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

        // stage the texture descriptors on the bindless heap
        // in bindless, MUST happen before setting descriptor (mp_pipeline->set_state)
        {
            const material& first_material = *batches[0u].m_material;
            vector<texture*> all_textures
            {
                backend.find_texture(first_material.get_texture_diffuse_name()),
                backend.find_texture(first_material.get_texture_normals_name())
            };

            for ()
            graphics::descriptor_range gpu_range = descriptor_manager.stage(all_textures);
            mp_pipeline->set_resource_table(commandlist, "bindless_heap", gpu_range);
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
        
        // stage the instance & vertex buffers
        {
            graphics::descriptor_range gpu_range = descriptor_manager.stage(m_instance_buffer_srv);
            mp_pipeline->set_resource_table(commandlist, "g_instancebuffer", gpu_range);

            gpu_range = descriptor_manager.stage(m_vertexbuffer.m_vertex_buffer_srv);
            mp_pipeline->set_resource_table(commandlist, "g_vertexbuffers", gpu_range);
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
            graphics::resource* index_buffer = batch.get_index_buffer();
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