#include "renderer_pch.h"
#include "quad_renderer.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/pipeline/pipeline.h"
#include "influx_renderer/resources/resource_manager.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

namespace influx::renderer
{
    struct gpu_perview final
    {
        math::matrix4x4f m_vp;
    };

    static graphics_pipeline_signature& get_pipeline_sig()
    {
        static graphics_pipeline_signature signature{};
        {
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::vs] = "quad_shaders::main_vs";
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::ps] = "quad_shaders::main_ps";

            signature.m_primitive_type = graphics::e_primitive_topology_type::triangle;
            signature.m_cullmode = graphics::e_cull_mode::nocull;
            signature.m_fillmode = graphics::e_fill_mode::solid;
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
            signature.m_blend_actives[0] = true;
            signature.m_blend_sources[0] = graphics::e_blend::src_alpha;
            signature.m_blend_dests[0] = graphics::e_blend::inv_src_alpha;
            signature.m_blend_ops[0] = graphics::e_blendop::add;
            signature.m_alpha_sources[0] = graphics::e_blend::one;
            signature.m_alpha_dests[0] = graphics::e_blend::zero;
            signature.m_alpha_ops[0] = graphics::e_blendop::add;
            signature.m_blend_writemasks[0] = 15u;
        }
        return signature;
    }

	quad_renderer::quad_renderer()
	{
        struct vertex final
        {
            math::float2 m_uv{};
        };
        
        vector<vertex> vertices =
        {
            {.m_uv{0.0f, 0.0f}},
            {.m_uv{1.0f, 0.0f}},
            {.m_uv{0.0f, 1.0f}},
            {.m_uv{1.0f, 1.0f}},
        };
        vector<index> indices = { 0u, 1u, 2u, 2u, 1u, 3u };

        renderer_backend& backend = renderer_backend::get_instance();
        resource_manager& resourceman = backend.get_resource_manager();

        mesh_data<vertex>* data = new mesh_data<vertex>{};
        data->m_vertices = vertices;
        data->m_indices = indices;

        const mesh_id id = make_id("quadrender_quad");
        mesh_resource& resource = resourceman.load<e_resource_type::mesh>(id, data, true);
        mp_vertexbuffer = resource.m_resource->m_vertexbuffer;
        mp_indexbuffer = resource.m_resource->m_indexbuffer;
	}

	quad_renderer::~quad_renderer()
	{
	}

	void quad_renderer::render_quad(graphics::commandlist* commandlist, const target& target)
	{
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipelineman = *backend.get_pipeline_manager();
        graphics_pipeline& pipeline = pipelineman.get_or_create_pipeline(get_pipeline_sig());
        
        logonce(e_log_category::warning, "influx::renderer::quad_renderer: first quad render!");

        pipeline.set_state(*commandlist);
        commandlist->set_primitive_topology(graphics::e_primitive_topology::trilist);
        commandlist->set_vertexbuffer(mp_vertexbuffer);
        commandlist->set_indexbuffer(mp_indexbuffer);
        commandlist->draw_indexed(
        {
            .m_num_indexes_per_instance{6u},
            .m_num_instances{1u},
            .m_start_index{0u},
            .m_start_vertex{0u},
            .m_start_instance{0u}
        });
	}
}