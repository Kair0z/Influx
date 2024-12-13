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
    pipeline::pipeline(graphics::device* device, renderer::shader_data const* vertex_shader, renderer::shader_data const* pixel_shader)
    {
        shader::reflection const* vs_reflection = vertex_shader ? &vertex_shader->m_reflection : nullptr;
        shader::reflection const* ps_reflection = pixel_shader ? &pixel_shader->m_reflection : nullptr;

        influx_assert(vertex_shader); // for now, we require vertex shaders!
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

        // parse resources in shader refletions
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
        if (vertex_shader)  pipeline_desc.m_vs = vertex_shader->m_bytecode;
        if (pixel_shader)   pipeline_desc.m_ps = pixel_shader->m_bytecode;
        // ...
        
        // depth stencil
        pipeline_desc.m_depth_stencil.m_depth_enable = true;
        pipeline_desc.m_depth_stencil.m_stencil_enable = false;

        // rasterizer
        pipeline_desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
        pipeline_desc.m_rasterizer.m_front_ccw = true;
        pipeline_desc.m_rasterizer;

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
        cmdlist->set(graphics::e_primitive_topology::trilist);
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

    void pipeline::save_to_file(const string& path) const
    {
        
    }
#endif
}
