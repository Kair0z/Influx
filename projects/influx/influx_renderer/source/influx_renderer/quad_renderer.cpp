#include "renderer_pch.h"
#include "quad_renderer.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/pipeline/pipeline.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

namespace influx::renderer
{
    struct gpu_perview final
    {
        math::matrix4x4f m_vp;
    };

    static const graphics_pipeline_signature k_quad_pipeline_signature
    {
        .m_shader_identifiers
        {
            "quad_shaders::main_vs",
            "quad_shaders::main_ps"
        },

        .m_primitive_type       { graphics_pipeline_signature::primitive_type::triangle },
        .m_cullmode             { graphics_pipeline_signature::cullmode::none },
        .m_fillmode             { graphics_pipeline_signature::fillmode::solid },
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
        .m_depth_format         { graphics_pipeline_signature::format::d32 },

        .m_rtv_actives          { true, false, false, false, false, false, false, false },
        .m_rtv_formats          { graphics_pipeline_signature::rgba8, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
        .m_blend_actives        { true, false, false, false, false, false, false, false },
        .m_blend_sources        { graphics_pipeline_signature::bl_src_alpha, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_dests          { graphics_pipeline_signature::bl_inv_src_alpha, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_ops            { graphics_pipeline_signature::op_add, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_sources        { graphics_pipeline_signature::bl_one, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_dests          { graphics_pipeline_signature::bl_zero, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_alpha_ops            { graphics_pipeline_signature::op_add, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
        .m_blend_writemasks     { graphics_pipeline_signature::blend_all, 15u, 15u, 15u, 15u, 15u, 15u, 15u }
    };

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
        mp_vertexbuffer = backend.create_vertexbuffer("quadrender_quad", vertices);
        mp_indexbuffer = backend.create_indexbuffer("quadrender_quad", indices);
	}

	quad_renderer::~quad_renderer()
	{
	}

	void quad_renderer::render_quad(graphics::commandlist* commandlist, const target& target)
	{
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipelineman = *backend.get_pipeline_manager();
        graphics_pipeline* pipeline = pipelineman.get_or_create_pipeline("pip_quad", k_quad_pipeline_signature);
        if (pipeline != nullptr)
        {
            logonce(e_log_category::warning, "influx::renderer::quad_renderer: first quad render!");

            pipeline->set_state(*commandlist);
            commandlist->set(graphics::e_primitive_topology::trilist);
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
}