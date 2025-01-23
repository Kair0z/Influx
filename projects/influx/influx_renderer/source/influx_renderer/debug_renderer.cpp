#include "renderer_pch.h"
#include "debug_renderer.h"

// influx::graphics
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/pipeline/pipeline.h"
#include "influx_renderer/renderer_common.h"

namespace influx::renderer
{
    struct debug_renderer::gpu_instance_data final
    {
        math::vectorf3	m_start_wp;
        math::vectorf3	m_end_wp;
        math::colour_rgba m_colour;
    };

    struct debug_renderer::gpu_perview final
    {
        math::matrix4x4f m_vp;
    };

    struct vertex final
    {
        math::float3 m_position;
        math::colour_rgba m_colour;
        uint32 m_id;
    };

    static const pipeline_signature k_debug_pipeline_signature
    {
        .m_vs_name              { "debug_shaders_vs" },
        .m_ps_name              { "debug_shaders_ps" },

        .m_primitive_type       { 2u }, // line
        .m_cullmode             { 2u }, // nocull
        .m_fillmode             { 0u }, // wireframe
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

        .m_depth_enable         { false },
        .m_stencil_enable       { false },
        .m_depth_comparison     { 0u },
        .m_depth_format         { 5u }, // d32

        .m_rtv_actives          { true, false, false, false, false, false, false, false },
        .m_rtv_formats          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
        .m_blend_actives        { false, false, false, false, false, false, false, false },
        .m_blend_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_sources        { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_dests          { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_ops            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_writemasks     { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u }
    };

    debug_renderer::debug_renderer(renderer_backend* backend, graphics::device* device, pipeline* pipeline)
        : mp_pipeline{pipeline}
        , mp_backend{backend}
        , mp_device{device}
    {
        m_instance_data.clear();
        m_instance_data.reserve(k_max_instances);
        
        // create instance buffer & srv
        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_instances * sizeof(gpu_instance_data);
            desc.m_bytestride = sizeof(gpu_instance_data);
            desc.m_init_state = graphics::e_resource_state::gen_read;
            mp_instancebuffer = device->create_resource(desc, heap_desc);
            mp_instancebuffer->set_name({ "debug_instance_buffer" });
            m_instance_buffer_srv = backend->get_descriptor_manager()->create_buffer_srv(mp_instancebuffer);
        }
        
        // create 2-element vertexbuffer
        {
            vector<vertex> vertices = {
                {.m_position{0.0f, 0.0f, 0.0f}, .m_colour{1,1,1,1}},
                {.m_position{1.0f, 0.0f, 0.0f}, .m_colour{1,1,1,1}}};

            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_init_state = graphics::e_resource_state::gen_read;
            desc.m_bytesize = vertices.size() * sizeof(vertex);
            desc.m_bytestride = sizeof(vertex);
            mp_vertexbuffer = mp_device->create_resource(desc, heap_desc);
            mp_vertexbuffer->map([&vertices](void* target)
            {
                memcpy(target, vertices.data(), vertices.size() * sizeof(vertex));
            });
        }

        m_gpu_perview = new gpu_perview();
    }

    debug_renderer::~debug_renderer()
    {
        delete m_gpu_perview;
    }

    void debug_renderer::render(graphics::commandlist* commandlist, const scene_debug& scene, const target& target)
    {
        // get the pipeline
        mp_pipeline = mp_backend->get_pipeline_manager()->get_or_create_pipeline("pip_debug", k_debug_pipeline_signature);
        if (mp_pipeline == nullptr)
        {
            return;
        }

        logonce(e_log_category::warning, "influx::renderer::debug_renderer: first debug render!");

        // update viewprojection matrix
        {
            const camera& camera = scene.m_camera;
            math::transform3D transform = camera.m_transform;
            transform.update_matrix();

            const float ar = (float)target.get_width() / target.get_height();
            m_gpu_perview->m_vp = make_viewprojection(transform.get_matrix(), ar, camera.m_fov, camera.m_near_plane, camera.m_far_plane);
        }

        mp_pipeline->set_state(commandlist);
        commandlist->set(graphics::e_primitive_topology::linelist);
        mp_pipeline->set_constants<gpu_perview>(commandlist, "g_perview", *m_gpu_perview);
        commandlist->set_vertexbuffer(mp_vertexbuffer);

        update_instance_buffer(scene);

        // stage the instance buffer and set as resource table
        const graphics::descriptor_range gpu_range = mp_backend->get_descriptor_manager()->stage(m_instance_buffer_srv);
        mp_pipeline->set_resource_table(commandlist, "g_instancebuffer", gpu_range);

        const uint32 num_instances = (uint32)m_instance_data.size();
        commandlist->draw_instanced(
        {
            .m_num_vertices_per_instance = 2u,
            .m_num_instances = num_instances,
            .m_start_vertex = 0u,
            .m_start_instance = 0u
        });
    }

    void debug_renderer::update_instance_buffer(const scene_debug& scene)
    {
        m_instance_data.clear();

        for (const scene_debug::line& line : scene.m_lines)
        {
            gpu_instance_data instance_data{};
            instance_data.m_colour = line.m_colour;
            instance_data.m_start_wp = line.m_points[0u];
            instance_data.m_end_wp = line.m_points[1u];
            m_instance_data.push_back(instance_data);
        }

        mp_instancebuffer->map([this](void* dest)
        {
            gpu_instance_data* data = reinterpret_cast<gpu_instance_data*>(dest);
            for (uint64 i = 0u; i < m_instance_data.size(); ++i)
            {
                data[i] = m_instance_data[i];
            }
        });
    }
}