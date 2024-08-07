#include "renderer_pch.h"
#include "renderer_imgui.h"

#include "imgui/imgui.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/upload_manager.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	imgui_manager::imgui_manager(graphics::device* device)
	{
		create_fonts_texture(device);
		create_pipeline(device);
	}

	void imgui_manager::create_fonts_texture(graphics::device* device)
	{
		renderer_backend& backend = renderer_backend::get_instance();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		// setup tex create args
		texture_create_args texture_args{};
		texture_args.m_width = width;
		texture_args.m_heigth = height;
		mp_fonts_texture = backend.create_texture(texture_args);

		// setup texture data
		texture_data tex_data{};
		for (size_t i = 0; i < mp_fonts_texture->get_num_pixels(); ++i)
		{
			tex_data.m_pixels.push_back(pixels[i]);
		}
		backend.upload_texture_data(mp_fonts_texture, tex_data);
	}

	void imgui_manager::create_pipeline(graphics::device* device)
	{
		// setup root signature
		graphics::rootsignature_desc rootsig_desc{};
		graphics::root_param_constants constants
		{
			16u, // num_dwords
			0u, // shader_reg
			0u,	// register_space
			graphics::e_shader_visibility::vertex
		};
		rootsig_desc.m_constants.push_back(constants);

		graphics::root_param_resource_range range
		{
			1u, // num_dwords
			graphics::root_param_resource_range::e_type::srv,
			0u, // shadder_reg
			0u	// register_space
		};
		rootsig_desc.m_resource_tables.push_back({ range });

		rootsig_desc.m_static_samplers.push_back(graphics::root_static_sampler
		{
			0u, // shader_register
			0u, // register_space
			graphics::e_shader_visibility::pixel,
			0.0f, // mip_lod_bias
			0.0f, // min_lod
			0.0f, // max_lod
			0u, // max_anisotropy
			graphics::e_texture_wrap_mode::wrap, // u
			graphics::e_texture_wrap_mode::wrap, // v
			graphics::e_texture_wrap_mode::wrap, // w
			graphics::e_filter::comparison_min_mag_mip_linear,
			graphics::e_comparison_func::always,
			graphics::e_border_color::black_transparent
		});

		mp_rootsig = device->create_rootsignature(rootsig_desc);

		graphics::pipeline_desc pipeline_desc{};
		pipeline_desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
		pipeline_desc.m_sample_mask = UINT_MAX;
		pipeline_desc.m_sample_count = 1u;
		pipeline_desc.m_rtvs[0].m_enabled = true;
		pipeline_desc.m_rtvs[0].m_format = graphics::e_format::rgba8;

		// pipeline layout
		pipeline_desc.add_input_element("POSITION", 0u, graphics::e_format::rg32, 0u, false, 0u);
		pipeline_desc.add_input_element("TEXCOORD", 0u, graphics::e_format::rg32, 0u, false, 0u);
		pipeline_desc.add_input_element("COLOR", 0u, graphics::e_format::rgba8, 0u, false, 0u);

		// blend setup
		pipeline_desc;

		// rasterizer
		pipeline_desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;

		mp_pipeline = device->create_pipeline(mp_rootsig, pipeline_desc);
	}
}