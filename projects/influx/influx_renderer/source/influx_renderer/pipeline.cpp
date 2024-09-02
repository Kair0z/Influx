#include "renderer_pch.h"
#include "pipeline.h"

// influx::renderer
#include "influx_renderer/texture.h"

// influx::graphics
#include "influx_graphics.h"
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	pipeline::pipeline(graphics::device* device, const renderer::shader_data& vertex_shader, const renderer::shader_data& pixel_shader)
	{
        // get the reflection data:
        const shader::reflection& vs_reflection = vertex_shader.m_reflection;
        const shader::reflection& ps_reflection = pixel_shader.m_reflection;

        constexpr static bool use_reflection = true;

        // build the root signature:
        graphics::rootsignature_desc rootsig_desc{};
        if (use_reflection)
        {
            auto reflect_resource = [&rootsig_desc, this]
            (const shader::reflection::resource& resource, graphics::e_shader_visibility shader_vis)
            {
                if (!resource.m_name.empty())
                    m_name_to_register[resource.m_name] = resource.m_shader_register;

                switch (resource.m_type)
                {
                case shader::reflection::resource::e_type::cbv:
                    rootsig_desc.add_root_constants(resource.m_bytesize / sizeof(uint32),
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

            for (const shader::reflection::resource& resource : vs_reflection.m_bound_resources)
            {
                reflect_resource(resource, graphics::e_shader_visibility::vertex);
            }
            for (const shader::reflection::resource& resource : ps_reflection.m_bound_resources)
            {
                reflect_resource(resource, graphics::e_shader_visibility::pixel);
            }
        }
        else
        {
            // resource table
            graphics::root_param_resource_range srv_range{};
            srv_range.m_num_resources = 128u;
            srv_range.m_register_space = 0u;
            srv_range.m_type = graphics::root_param_resource_range::e_type::srv;

            graphics::root_param_resource_table table{};
            table.m_resource_ranges.push_back(srv_range);
            table.m_common.m_visibility = graphics::e_shader_visibility::pixel;
            rootsig_desc.m_resource_tables.push_back(table);

            // constants
            rootsig_desc.m_constants.push_back({ 16u, 0u, 0u, graphics::e_shader_visibility::vertex }); // _per_frame_vs
            rootsig_desc.m_constants.push_back({ 1u, 0u, 0u, graphics::e_shader_visibility::pixel }); // _per_frame_ps

            // static samplers
            rootsig_desc.m_static_samplers.push_back({});
        }

        mp_rootsig = device->create_rootsignature(rootsig_desc);
        influx_assert(mp_rootsig->is_valid());

        m_name_to_param_idx = mp_rootsig->get_param_idx_table();

        // build the pipeline
        graphics::pipeline_desc pipeline_desc{};
        pipeline_desc.m_vs = vertex_shader.m_bytecode;
        pipeline_desc.m_ps = pixel_shader.m_bytecode;

        pipeline_desc.m_depth_stencil.m_depth_enable = false;
        pipeline_desc.m_depth_stencil.m_stencil_enable = false;

        pipeline_desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
        pipeline_desc.m_rasterizer.m_front_ccw = false;
        
        if (use_reflection)
        {
            // derive the input elements from reflection:
            for (uint32 i = 0u; i < vs_reflection.m_input_params.size(); ++i)
            {
                shader::reflection::input_param param = vs_reflection.m_input_params[i];

                // derive the format here:
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
        }
        else
        {
            pipeline_desc.add_input_element("POSITION", 0u, graphics::e_format::rgb32, 0u, false, 0u);
            pipeline_desc.add_input_element("COLOR", 0u, graphics::e_format::rgba32, 0u, false, 0u);
            pipeline_desc.add_input_element("NORMAL", 0u, graphics::e_format::rgb32, 0u, false, 0u);
            pipeline_desc.add_input_element("TEXCOORD", 0u, graphics::e_format::rg32, 0u, false, 0u);
        }

        mp_pipeline = device->create_pipeline(mp_rootsig, pipeline_desc);
        influx_assert(mp_pipeline->is_valid());
	}

    void pipeline::set_state(graphics::command_list* cmdlist)
    {
        cmdlist->set(mp_rootsig);
        cmdlist->set(mp_pipeline);
        cmdlist->set(graphics::e_primitive_topology::trilist);
    }

    void pipeline::set_constants(graphics::command_list* cmdlist, const string& name, uint32 num_dwords, void* data)
    {
        uint32 param_idx = get_param_index(name);
        cmdlist->set_constants(param_idx, num_dwords, data);
    }

    void pipeline::set_resource_table(graphics::command_list* cmdlist, const string& name, const graphics::descriptor_range& gpu_range)
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

    void pipeline::set_name(const string& name)
    {
        mp_pipeline->set_name(name);
    }

    const string& pipeline::get_name() const
    {
        return mp_pipeline->get_name();
    }
}
