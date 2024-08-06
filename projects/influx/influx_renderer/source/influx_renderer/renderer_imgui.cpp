#include "renderer_pch.h"
#include "renderer_imgui.h"

#include "influx_graphics/device.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	imgui_manager::imgui_manager(graphics::device* device, texture* fonts_texture)
	{
		create_fonts_texture(device, fonts_texture);
		create_pipeline(device);
	}

	void imgui_manager::create_fonts_texture(graphics::device* device, texture* fonts_texture)
	{
		mp_fonts_texture = fonts_texture;
		// todo...
#if 0
		texture& fonts_tex = get_texfonts();

		// get texture data:
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		uint32 tex_width = width;
		uint32 tex_height = height;
		size_t tex_pitch = tex_width * sizeof(unsigned char);
		size_t tex_bytesize = tex_height * tex_pitch;

		graphics::e_format tex_format = graphics::e_format::rgba8;

		// upload texture to graphics:
		graphics::buffer_desc buffer_desc{};
		buffer_desc.m_bytesize = tex_bytesize;
		buffer_desc.m_init_state = graphics::e_resource_state::copy_source;

		graphics::tex2D_desc texture_desc{};
		texture_desc.m_dimensions = { tex_width, tex_height };
		texture_desc.m_format = tex_format;
		texture_desc.m_init_state = graphics::e_resource_state::copy_dest;

		fonts_tex.mp_upload = get_device()->create_resource(buffer_desc, { graphics::e_heap_type::shared });
		fonts_tex.mp_resource = get_device()->create_resource(texture_desc);

		// texture data -> upload res
		fonts_tex.mp_upload->map([tex_pitch, tex_height, pixels](void* dest)
		{
			for (uint32 y = 0; y < tex_height; ++y)
			memcpy((void*)((uintptr_t)dest + y * tex_pitch),
				pixels + y * tex_pitch, tex_pitch);
		});

		// record transfer (upload resource -> gpu resource)
		get_commandlist()->start(get_allocator());
		get_commandlist()->copy_texture(
			fonts_tex.mp_upload, fonts_tex.mp_resource);
		get_commandlist()->end();

		// submit transfer
		get_queue()->submit_commandlists({ get_commandlist() });
		get_queue()->queue_signal(get_fence(), 1u);

		// wait for transfer to finish on gpu
		wait_handle wait{};
		get_fence()->wait_for_value(1u, wait);

		// create srv
		fonts_tex.mp_srv = get_device()->create_srv(get_srv_heap(), fonts_tex.mp_resource);
#endif
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