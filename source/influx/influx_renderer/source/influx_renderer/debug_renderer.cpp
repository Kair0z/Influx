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
#include "influx_renderer/resources/resource_manager.h"

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

    static graphics_pipeline_signature& get_pipeline_sig()
    {
        static graphics_pipeline_signature signature{};
        {
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::vs] = "debug_shaders::main_vs";
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::ps] = "debug_shaders::main_ps";

            signature.m_primitive_type = graphics::e_primitive_topology_type::line;
            signature.m_cullmode = graphics::e_cull_mode::nocull;
            signature.m_fillmode = graphics::e_fill_mode::wireframe;
            signature.m_forced_samplecount = 0u;
            signature.m_sample_mask = (uint32)-1;
            signature.m_sample_count = 1u;
            signature.m_front_ccw = false;
            signature.m_depthclip = false;
            signature.m_multisample = false;
            signature.m_antialiased_line = false;
            signature.m_conservative_raster = false;
            signature.m_depthbias = 0;
            signature.m_depthbias_clamp = 0.0f;
            signature.m_slope_depthbias = 0.0f;

            signature.m_depth_enable = false;
            signature.m_stencil_enable = false;
            signature.m_depth_comparison = graphics::e_comparison_func::less;
            signature.m_depth_format = graphics::e_format::d32;

            signature.m_rtv_actives[0] = true;
            signature.m_rtv_formats[0] = graphics::e_format::rgba8;

            signature.m_blend_actives[0] = false;
        }
        return signature;
    }

    debug_renderer::debug_renderer()
    {
        renderer_backend& backend = renderer_backend::get_instance();
        graphics::device& device = backend.get_device();

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
            mp_instancebuffer = device.create_resource(desc, heap_desc);
            mp_instancebuffer->set_name({ "debug_instance_buffer" });
            m_instance_buffer_srv = backend.get_descriptor_manager()->create_buffer_srv(mp_instancebuffer);
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
            mp_vertexbuffer = device.create_resource(desc, heap_desc);
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

    void debug_renderer::render(graphics::commandlist* commandlist, const scene& scene, const target& target)
    {
        // get the pipeline
        renderer_backend& backend = renderer_backend::get_instance();
        graphics_pipeline& pipeline = backend.get_pipeline_manager()->get_or_create_pipeline(get_pipeline_sig());
        descriptor_manager& descriptorman = *backend.get_descriptor_manager();

        logonce(e_log_category::warning, "influx::renderer::debug_renderer: first debug render!");

        // update viewprojection matrix
        m_gpu_perview->m_vp = scene.get_view_matrices().m_viewprojection;

        pipeline.set_state(*commandlist);
        commandlist->set(graphics::e_primitive_topology::linelist);
        pipeline.set_constants<gpu_perview>(*commandlist, "g_perview", *m_gpu_perview);
        commandlist->set_vertexbuffer(mp_vertexbuffer);

        update_instance_buffer(scene);

        // stage the instance buffer and set as resource table
        const graphics::descriptor_range gpu_range = descriptorman.stage(m_instance_buffer_srv);
        pipeline.set_resource_table(*commandlist, "g_instancebuffer", gpu_range);

        const uint32 num_instances = (uint32)m_instance_data.size();
        commandlist->draw_instanced(
        {
            .m_num_vertices_per_instance = 2u,
            .m_num_instances = num_instances,
            .m_start_vertex = 0u,
            .m_start_instance = 0u
        });
    }

    bool debug_renderer::can_build_pipeline() const
    {
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipelineman = *backend.get_pipeline_manager();
        resource_manager& resourceman = backend.get_resource_manager();
        
        bool has_all_shaders = true;
        
        const auto& pipeline_signature = get_pipeline_sig();
        for (const shader::shader_signature& shadersig : pipeline_signature.get_shader_signatures())
        {
            const bool is_shader_optional = pipeline_signature.is_shader_optional(shadersig.m_type);
            if (resourceman.contains<e_resource_type::shader>(shadersig) == false && !is_shader_optional)
            {
                // ... missing shader
                has_all_shaders = false;
            }
        }

        return has_all_shaders;
    }

    void debug_renderer::update_instance_buffer(const scene& scene)
    {
        m_instance_data.clear();

        for (const line& line : scene.get_lines())
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