#include "renderer_pch.h"
#include "pipeline.h"

// influx::renderer
#include "influx_renderer/texture.h"

// influx::graphics
#include "influx_graphics.h"
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/descriptors.h"

namespace influx::renderer
{
    pipeline::pipeline(graphics::device* device, const pipeline_signature& signature, renderer::shader_data const* vs, renderer::shader_data const* ps)
    {
        m_signature = signature;

        shader::reflection const* vs_reflection = vs ? &vs->m_reflection : nullptr;
        shader::reflection const* ps_reflection = ps ? &ps->m_reflection : nullptr;

        influx_assert(vs); // for now, we require vertex shaders!
        influx_assert(vs_reflection);

        // build the root signature:
        graphics::rootsignature_desc& rootsig_desc = m_rootsig_desc;
        auto reflect_resource = [&rootsig_desc, this]
        (const shader::reflection::resource& resource, graphics::e_shader_visibility shader_vis)
        {
            if (!resource.m_name.empty())
                m_name_to_register[resource.m_name] = resource.m_shader_register;

            switch (resource.m_type)
            {
            case shader::reflection::resource::e_type::cbv:
                rootsig_desc.add_root_constants((uint32)resource.m_bytesize / sizeof(uint32),
                    resource.m_shader_register, resource.m_register_space, shader_vis);
                rootsig_desc.name_last_constants(resource.m_name);
                break;

            case shader::reflection::resource::e_type::structured:
                rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
                    resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
                rootsig_desc.name_last_resource_table(resource.m_name);
                break;

            case shader::reflection::resource::e_type::texture:
                rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
                    resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
                rootsig_desc.name_last_resource_table(resource.m_name);
                break;

            case shader::reflection::resource::e_type::sampler:
                rootsig_desc.add_root_sampler(resource.m_shader_register, resource.m_register_space, shader_vis);
                rootsig_desc.name_last_sampler(resource.m_name);
                break;
            }
        };

        // parse resources in shader reflections
        if (vs_reflection)
        {
            for (const shader::reflection::resource& resource : vs_reflection->m_bound_resources)
            {
                reflect_resource(resource, graphics::e_shader_visibility::vertex);
            }
        }
        if (ps_reflection)
        {
            for (const shader::reflection::resource& resource : ps_reflection->m_bound_resources)
            {
                reflect_resource(resource, graphics::e_shader_visibility::pixel);
            }
        }

        // create root signature
        mp_rootsig = device->create_rootsignature(rootsig_desc);
        m_name_to_param_idx = mp_rootsig->get_param_idx_table();
        influx_assert(mp_rootsig->is_valid());

        // build the pipeline
        graphics::pipeline_desc& pipeline_desc = m_create_desc;
        if (vs)  pipeline_desc.m_vs = vs->m_bytecode;
        if (ps)   pipeline_desc.m_ps = ps->m_bytecode;
        // ...

        pipeline_desc.m_rasterizer.m_cullmode = (graphics::e_cull_mode)signature.m_cullmode;
        pipeline_desc.m_rasterizer.m_front_ccw = signature.m_front_ccw;
        pipeline_desc.m_prim_type = (graphics::e_primitive_topology_type)signature.m_primitive_type;
        pipeline_desc.m_rasterizer.m_fillmode = (graphics::e_fill_mode)signature.m_fillmode;
        pipeline_desc.m_rasterizer.m_forced_samplecount = signature.m_forced_samplecount;
        pipeline_desc.m_sample_mask = signature.m_sample_mask;
        pipeline_desc.m_sample_count = signature.m_sample_count;
        pipeline_desc.m_rasterizer.m_depth_clip_enable = signature.m_depthclip;
        pipeline_desc.m_rasterizer.m_multisample = signature.m_multisample;
        pipeline_desc.m_rasterizer.m_antialiased_line = signature.m_antialiased_line;
        pipeline_desc.m_rasterizer.m_conservative = signature.m_conservative_raster;
        pipeline_desc.m_rasterizer.m_depth_bias = signature.m_depthbias;
        pipeline_desc.m_rasterizer.m_depth_bias_clamp = signature.m_depthbias_clamp;
        pipeline_desc.m_rasterizer.m_slope_depth_bias = signature.m_slope_depthbias;
        pipeline_desc.m_rasterizer.m_front_ccw = true;
        pipeline_desc.m_depth_stencil.m_depth_enable = signature.m_depth_enable;
        pipeline_desc.m_depth_stencil.m_stencil_enable = signature.m_stencil_enable;
        pipeline_desc.m_depth_stencil.m_depth_func = (graphics::e_comparison_func)signature.m_depth_comparison;
        pipeline_desc.m_format_dsv = (graphics::e_format)signature.m_depth_format;
        for (uint8 i = 0u; i < 8u; ++i)
        {
            pipeline_desc.m_blends[i].m_enabled = signature.m_blend_actives[i];
            pipeline_desc.m_blends[i].m_src = (graphics::e_blend)signature.m_blend_sources[i];
            pipeline_desc.m_blends[i].m_dest = (graphics::e_blend)signature.m_blend_dests[i];
            pipeline_desc.m_blends[i].m_op = (graphics::e_blendop)signature.m_blend_ops[i];
            pipeline_desc.m_blends[i].m_srcalpha = (graphics::e_blend)signature.m_alpha_sources[i];
            pipeline_desc.m_blends[i].m_destalpha = (graphics::e_blend)signature.m_alpha_dests[i];
            pipeline_desc.m_blends[i].m_op_alpha = (graphics::e_blendop)signature.m_alpha_ops[i];
            pipeline_desc.m_blends[i].m_write_mask = signature.m_blend_writemasks[i];
            pipeline_desc.m_rtvs[i].m_enabled = signature.m_rtv_actives[i];
            pipeline_desc.m_rtvs[i].m_format = (graphics::e_format)signature.m_rtv_formats[i];
        }

        // parse the input elements from reflection:
        for (uint32 i = 0u; i < vs_reflection->m_input_params.size(); ++i)
        {
            const shader::reflection::input_param& param = vs_reflection->m_input_params[i];

            // derive the format
            graphics::e_format format;
            switch (param.m_num_floats)
            {
            case 1u: format = graphics::e_format::r32; break;
            case 2u: format = graphics::e_format::rg32; break;
            case 3u: format = graphics::e_format::rgb32; break;
            case 4u: format = graphics::e_format::rgba32; break;
            default:
                influx_assert(false); // WOAH!
                break;
            }

            pipeline_desc.add_input_element(
                param.m_semantic_name,
                param.m_semantic_index,
                format,
                0u,
                false,
                0u);
        }

        mp_pipeline = device->create_pipeline(mp_rootsig, pipeline_desc);
        influx_assert(mp_pipeline->is_valid());
    }

    pipeline* pipeline::load_from_file(const string& path)
    {
        return nullptr;
    }

    void pipeline::set_state(graphics::commandlist* cmdlist)
    {
        cmdlist->set(mp_rootsig);
        cmdlist->set(mp_pipeline);
    }

    void pipeline::set_constants(graphics::commandlist* cmdlist, const string& name, uint32 num_dwords, void* data)
    {
        uint32 param_idx = get_param_index(name);
        cmdlist->set_constants(param_idx, num_dwords, data);
    }

    void pipeline::set_resource_table(graphics::commandlist* cmdlist, const string& name, const graphics::descriptor_range& gpu_range)
    {
        uint32 param_idx = get_param_index(name);
        cmdlist->set(gpu_range, param_idx);
    }

    uint32 pipeline::get_shader_register(const string& resource_name)
    {
        return m_name_to_register[resource_name];
    }

    uint32 pipeline::get_param_index(const string& resource_name)
    {
        return m_name_to_param_idx[resource_name];
    }

#if INFLUX_DEBUG
    void pipeline::set_name(const string& name)
    {
        mp_pipeline->set_name(name);
    }

    const string& pipeline::get_name() const
    {
        return mp_pipeline->get_name().get();
    }
#endif

    void pipeline::save_to_file(const string& path) const
    {

    }
    const pipeline_signature& pipeline::get_signature() const
    {
        return m_signature;
    }
}
