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
    static const pipeline_signature k_scene_pipeline_signature
    {
        .m_vs_name              { "shaders_vs" },
        .m_ps_name              { "shaders_ps" },

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
        .m_depth_format         { pipeline_signature::format::d32 }, // d32

        .m_rtv_actives          { true, false, false, false, false, false, false, false },
        .m_rtv_formats          { pipeline_signature::format::rgba8, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
        .m_blend_actives        { false, false, false, false, false, false, false, false },
        .m_blend_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_writemasks     { 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u }
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
        desc.m_init_state = graphics::e_resource_state::read;
        mp_instancebuffer = device->create_resource(desc, heap_desc);
        mp_instancebuffer->set_name({ "scene_instance_buffer" });

        // create srv
        mp_instance_buffer_srv = backend->get_descriptor_manager()->create_buffer_srv(mp_instancebuffer);
    }

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

    batch::batch(
        const string& mesh_name, 
        const string& material_name, 
        const vector<gpu_instance_data>& instances,
        uint32 instance_base)
        : m_instances{ instances }
        , m_base_instance{ instance_base }
    {
        auto& backend = renderer_backend::get_instance();

        backend.get_mesh_buffers(mesh_name, m_vertex_buffer, m_index_buffer);
        m_material = backend.get_material(material_name);
        influx_assert(m_material != nullptr);
    }

    graphics::resource* batch::get_vertex_buffer() const
    {
        return m_vertex_buffer;
    }

    graphics::resource* batch::get_index_buffer() const
    {
        return m_index_buffer;
    }

    material* batch::get_material() const
    {
        return m_material;
    }

    const vector<gpu_instance_data>& batch::get_instances() const
    {
        return m_instances;
    }

    const uint32 batch::get_instance_base() const
    {
        return m_base_instance;
    }

    vector<batch> scene_renderer::create_batches(const scene& scene)
    {
        // group instances per material, then per mesh
        using per_material_instances = umap<string, vector<gpu_instance_data>>;
        umap<string, per_material_instances> instances_per_mesh_per_material{};

        for (const string& mesh_name : mp_backend->get_mesh_names())
        {
            for (const mesh_instance& instance : scene.m_meshes)
            {
                if (instance.m_name == mesh_name)
                {
                    const string& material_name = instance.m_material_name.empty() ? "none" : instance.m_material_name;
                    per_material_instances& instances_per_material = instances_per_mesh_per_material[mesh_name];

                    // mesh_instance --> gpu_instance_data
                    gpu_instance_data instance_data{};
                    instance_data.m_transform = instance.m_transform;
                    instance_data.m_colour = instance.m_per_instance_colour;
                    instance_data.m_invert_normals = instance.m_invert_normals;
                    instances_per_material[material_name].push_back(instance_data);
                }
            }
        }

        vector<batch> batches{};
        uint32 instance_offset = 0u;
        for (const auto& per_mesh : instances_per_mesh_per_material)
        {
            const string& mesh_name = per_mesh.first;
            for (const auto& per_material : per_mesh.second)
            {
                const string& material_name = per_material.first;
                batches.push_back(batch(mesh_name, material_name, per_material.second, instance_offset));
                instance_offset += (uint32)per_material.second.size();
            }
        }

        return batches;
    }

    void scene_renderer::update_instance_buffer(const vector<batch>& batches)
    {
        mp_instancebuffer->map([&batches](void* dest)
        {
            gpu_instance_data* data = reinterpret_cast<gpu_instance_data*>(dest);
            for (const batch& batch : batches)
            {
                const auto& instances = batch.get_instances();
                for (size_t i = 0u; i < instances.size(); ++i)
                {
                    data[batch.get_instance_base() + i] = instances[i];
                }
            }
        });
    }

    void scene_renderer::render_batches(graphics::commandlist* commandlist, const vector<batch>& batches)
    {
        renderer_backend& backend = renderer_backend::get_instance();

        // stage the instance buffer
        {
            graphics::descriptor_range gpu_range =
                backend.get_descriptor_manager()->stage(mp_instance_buffer_srv->get_cpu_handle());

            // set the resource table
            mp_pipeline->set_resource_table(commandlist, "g_instancebuffer", gpu_range);
        }
        
        for (const batch& batch : batches)
        {
            material* material = batch.get_material();

            // find the material textures
            vector<texture*> material_textures(4u);
            material_textures[0] = backend.get_texture(material->m_tex_albedo);
            material_textures[1] = backend.get_texture(material->m_tex_normal);
            material_textures[2] = backend.get_texture(material->m_tex_roughness);
            material_textures[3] = backend.get_texture(material->m_tex_special);

            // stage the descriptors onto the gpu-visible heap
            graphics::descriptor_range gpu_range = backend.get_descriptor_manager()->stage(material_textures);
            mp_pipeline->set_resource_table(commandlist, "g_textures", gpu_range);

            // set index / vertex buffers
            graphics::resource* vertex_buffer = batch.get_vertex_buffer();
            graphics::resource* index_buffer = batch.get_index_buffer();
            const uint32 num_vertices = (uint32)vertex_buffer->get_bytesize() / (uint32)vertex_buffer->get_bytestride();
            const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();
            commandlist->set_indexbuffer(index_buffer);
            commandlist->set_vertexbuffer(vertex_buffer);

            // set base instance variable
            m_gpu_perdraw.m_start_instance = batch.get_instance_base();
            m_gpu_permaterial.m_colour = material->m_basecolor;

            mp_pipeline->set_constants(commandlist, "g_permaterial", m_gpu_permaterial);
            mp_pipeline->set_constants(commandlist, "g_perdraw", m_gpu_perdraw);

            const vector<gpu_instance_data>& instances = batch.get_instances();
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

    void scene_renderer::render_shadows(graphics::commandlist* commandlist, 
        const scene& scene, const vector<batch>& batches)
    {
        // mp_shadowspipeline = mp_backend->get_pipeline_manager()->get_pipeline("pip_shadows");
        influx_assert(mp_shadowspipeline);

        mp_shadowspipeline->set_state(commandlist);

        // set shadowtarget dsv
        commandlist->set(nullptr, mp_shadowstarget->get_dsv());

        // push constants
        math::transform3D light_transform = scene.m_camera.m_transform;
        const math::matrix4x4f mat_view = light_transform.get_matrix().inverted();
        //const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(camera.m_fov, (float)target.get_width() / target.get_height(), camera.m_near_plane, camera.m_far_plane);
        m_gpu_perview.m_vp = mat_view;
        mp_shadowspipeline->set_constants<gpu_perview>(commandlist, "g_perview", m_gpu_perview);

        // render
        render_batches(commandlist, batches);
    }

    void scene_renderer::render_basepass(graphics::commandlist* commandlist, 
        const scene& scene, const vector<batch>& batches, const target& target)
    {
        mp_pipeline = mp_backend->get_pipeline_manager()->get_or_create_pipeline("pip_scene", k_scene_pipeline_signature);
        if (mp_pipeline == nullptr)
        {
            return;
        }

        logonce(e_log_category::warning, "influx::renderer::scene_renderer: first scene render!");

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        mp_pipeline->set_state(commandlist);
        commandlist->set(graphics::e_primitive_topology::trilist);

        // update viewprojection matrix
        {
            const camera& camera = scene.m_camera;
            math::transform3D transform = camera.m_transform;
            transform.update_matrix();

            const float ar = (float)target.get_width() / target.get_height();
            m_gpu_perview.m_vp = make_viewprojection(transform.get_matrix(), ar, camera.m_fov, camera.m_near_plane, camera.m_far_plane);
        }
        
        mp_pipeline->set_constants<gpu_perscene>(commandlist, "g_perscene", m_gpu_perscene);
        mp_pipeline->set_constants<gpu_perview>(commandlist, "g_perview", m_gpu_perview);

        render_batches(commandlist, batches);
    }

    void scene_renderer::render(graphics::commandlist* commandlist, const scene& scene, const target& target)
    {
        m_gpu_perscene.m_delta_seconds = scene.m_delta_seconds;
        m_gpu_perscene.m_seconds = scene.m_seconds;

        // setup a batched draw
        vector<batch> batches = create_batches(scene);

        update_instance_buffer(batches);

        // render_shadows(commandlist, scene, batches);
        render_basepass(commandlist, scene, batches, target);
    }
}