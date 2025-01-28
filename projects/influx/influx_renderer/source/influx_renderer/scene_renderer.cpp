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

        descriptor_manager& descriptor_manager = *backend->get_descriptor_manager();
        m_sampler_view = descriptor_manager.create_sampler();
    }

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

#pragma region batch
    // 1 draw-call == 1 batch
    class batch final
    {
    public:
        string m_mesh_name;
        string m_material_name;
        vector<gpu_instance_data> m_instances;
        graphics::resource* m_vertexbuffer;
        graphics::resource* m_indexbuffer;
        uint64 m_base_instance{};
    };
#pragma endregion

    inline gpu_instance_data translate(const mesh_instance& mesh)
    {
        gpu_instance_data instance_data{};
        instance_data.m_transform = mesh.m_transform;
        instance_data.m_colour = mesh.m_per_instance_colour;
        return instance_data;
    }

    vector<batch> scene_renderer::create_batches(const scene& scene, graphics::commandlist* commandlist)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        umap<texture*, uint32> tex_to_idx{};

        // stage all srv/uav/const descriptors on the bindless heap
        // in bindless, MUST happen before setting descriptor (mp_pipeline->set_state)
        {
            vector<graphics::descriptor_handle> all_srvs{};
            all_srvs.push_back(m_instance_buffer_srv);

            uint32 texture_count = 0u;
            for (const mesh_instance& instance : scene.m_meshes)
            {
                const material& material = *instance.m_material;
                texture* diffuse_texture = backend.find_texture(material.get_texture_diffuse_name());
                if (diffuse_texture != nullptr && !tex_to_idx.contains(diffuse_texture))
                {
                    // add to list of unique srvs
                    all_srvs.push_back(diffuse_texture->get_cpu_handle());

                    // keep the idx of the srv
                    tex_to_idx[diffuse_texture] = texture_count;
                    texture_count++;
                }
            }

            graphics::descriptor_range gpu_range = backend.get_descriptor_manager()->stage(all_srvs);
            graphics::descriptor_range gpu_range_samp = backend.get_descriptor_manager()->stage_sampler(m_sampler_view);
            // mp_pipeline->set_resource_table(commandlist, "all_descriptors", gpu_range);
            backend.get_descriptor_manager()->start_commandlist(commandlist);
        }

        using mesh_to_instance_map = umap<string, vector<gpu_instance_data>>;
        mesh_to_instance_map meshname_to_instances{};

        for (const mesh_instance& instance : scene.m_meshes)
        {
            graphics::resource* indexbuffer = nullptr;
            graphics::resource* vertexbuffer = nullptr;

            gpu_instance_data gpu_data = translate(instance);
            gpu_data.m_albedo_index = tex_to_idx[backend.find_texture(instance.m_material->get_texture_diffuse_name())];
            meshname_to_instances[instance.m_name].push_back(gpu_data);
        }

        vector<batch> batches{};
        uint64 offset = 0u;
        for (const auto& pair : meshname_to_instances)
        {
            graphics::resource* vertexbuffer = nullptr;
            graphics::resource* indexbuffer = nullptr;
            if (backend.get_mesh_buffers(pair.first, vertexbuffer, indexbuffer))
            {
                batches.push_back({});
                batch& batch = batches.back();

                batch.m_indexbuffer = indexbuffer;
                batch.m_vertexbuffer = vertexbuffer;
                batch.m_instances = pair.second;
                batch.m_base_instance = offset;

                offset += batch.m_instances.size();
            }
        }
        return batches;
    }

    void scene_renderer::gather_textures(const scene& scene)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        graphics::descriptor_handle blank_descriptor = backend.get_default_texture().get_cpu_handle();

        
    }

    void scene_renderer::update_instance_buffer(const vector<batch>& batches)
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
    }

    void scene_renderer::render_basepass(graphics::commandlist* commandlist, 
        const scene& scene, const vector<batch>& batches, const target& target)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        descriptor_manager& descriptor_manager = *backend.get_descriptor_manager();

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
            // set constants
            m_gpu_perdraw.m_start_instance = batch.m_base_instance;
            m_gpu_permaterial.m_colour = {};

            mp_pipeline->set_constants(commandlist, "g_permaterial", m_gpu_permaterial);
            mp_pipeline->set_constants(commandlist, "g_perdraw", m_gpu_perdraw);

            // bind index buffer
            graphics::resource* index_buffer = batch.m_indexbuffer;
            const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();
            commandlist->set_indexbuffer(index_buffer);
            commandlist->set_vertexbuffer(batch.m_vertexbuffer);
            influx_assert(batch.m_vertexbuffer->get_bytestride() == 48u);
            const uint32 num_instances = (uint32)batch.m_instances.size();
            commandlist->draw_indexed(
            {
                .m_num_indexes_per_instance = num_indices,
                .m_num_instances = num_instances,
                .m_start_index = 0u,
                .m_start_vertex = 0u,
                .m_start_instance = m_gpu_perdraw.m_start_instance
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
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipeline_man = *backend.get_pipeline_manager();

        apply_pipeline_settings();

        mp_pipeline = pipeline_man.get_or_create_pipeline("pip_scene", k_scene_pipeline_signature);
        if (mp_pipeline == nullptr || scene.is_empty())
        {
            return;
        }

        // per-scene
        m_gpu_perscene.m_time.x = scene.m_delta_seconds;
        m_gpu_perscene.m_time.y = scene.m_seconds;

        // create batches
        vector<batch> batches = create_batches(scene, commandlist);

        update_instance_buffer(batches);

        logonce(e_log_category::warning, "influx::renderer::scene_renderer: first scene render!");

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        {
            mp_pipeline->set_state(commandlist);
            commandlist->set(graphics::e_primitive_topology::trilist);
        }

        // render_shadows(commandlist, scene, batches);
        render_basepass(commandlist, scene, batches, target);
    }
}