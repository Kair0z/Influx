#include "renderer_pch.h"
#include "renderer_imgui.h"

#include "imgui/imgui.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/descriptor_manager.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/queue.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/commandlist.h"

// influx::shader
#include "influx_shader.h"

#pragma region shaders
static const char* k_vertex_shader =
"cbuffer vertexBuffer : register(b0) \
            {\
              float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
              float2 pos : POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
              PS_INPUT output;\
              output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
              output.col = input.col;\
              output.uv  = input.uv;\
              return output;\
            }";

static const char* k_pixel_shader =
"struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            SamplerState sampler0 : register(s0);\
            Texture2D texture0 : register(t0);\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
              float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
              return out_col; \
            }";
#pragma endregion

namespace influx::renderer
{
	imgui_manager::imgui_manager(graphics::device* device)
		: mp_device{device}
	{
		create_fonts_texture(device);
		create_pipeline(device);
	}

	void imgui_manager::render(graphics::commandlist* commandlist, ImDrawData* draw_data, const target& target)
	{
		// Avoid rendering when minimized
		if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			return;

		if (draw_data->CmdListsCount <= 0)
			return;

		// update vertex / index buffers
		update_buffers(draw_data);

		const math::vectorf2& target_dim = { target.get_width(), target.get_height() };

		graphics::viewport viewport{};
		viewport.m_width = target_dim.x;
		viewport.m_height = target_dim.y;
		viewport.m_depth_min = 0.0f;
		viewport.m_depth_max = 1.0f;

		struct vertex_const_buffer
		{
			float  m_mvp[4][4];
		};
		vertex_const_buffer vertex_constant_buffer;
		{
			float L = draw_data->DisplayPos.x;
			float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
			float T = draw_data->DisplayPos.y;
			float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
			float mvp[4][4] =
			{
				{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
				{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
				{ 0.0f,         0.0f,           0.5f,       0.0f },
				{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
			};
			memcpy(&vertex_constant_buffer.m_mvp, mvp, sizeof(mvp));
		}

		// setup state
		renderer_backend& backend = renderer_backend::get_instance();
		descriptor_manager& descriptor_manager = *backend.get_descriptor_manager();
		commandlist->set_vertexbuffer(mp_vertexbuffer);
		commandlist->set_indexbuffer(mp_indexbuffer);
		commandlist->set(viewport);
		commandlist->set(graphics::e_primitive_topology::trilist);
		commandlist->set(mp_pipeline);
		commandlist->set(mp_rootsig);
		commandlist->set_constants(0u, 16u, &vertex_constant_buffer);

		// stage the font srv onto the gpu heap
		graphics::descriptor_range font_gpu_range = descriptor_manager.stage(mp_fonts_texture->get_srv());
		graphics::descriptor_range tex_gpu_range = font_gpu_range;

		// setup draw
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		int global_vtx_offset = 0;
		int global_idx_offset = 0;
		ImVec2 clip_off = draw_data->DisplayPos;
		for (int n = 0; n < draw_data->CmdListsCount; ++n)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
				ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				// Apply Scissor/clipping rectangle, Bind texture, Draw
				graphics::rect rect
				{
					.m_left = (uint32)clip_min.x,
					.m_top = (uint32)clip_min.y,
					.m_right = (uint32)clip_max.x,
					.m_bottom = (uint32)clip_max.y,
				};
				commandlist->set(rect);

				// if this command has a bound TexID (texture*/void*),
				// we should stage the texture (allocate gpu descriptor)
				// and bind that range to the commandlist
				const bool command_has_texture = pcmd->GetTexID() != 0u;
				if (command_has_texture)
				{
					texture* tex = reinterpret_cast<texture*>(pcmd->GetTexID());
					if (tex != nullptr)
					{
						tex_gpu_range = descriptor_manager.stage(tex);
					}
				}
				else
				{
					tex_gpu_range = font_gpu_range;
				}

				commandlist->set(tex_gpu_range, 1u);
				commandlist->draw_indexed({
					.m_num_indexes_per_instance = pcmd->ElemCount,
					.m_num_instances = 1u,
					.m_start_index = pcmd->IdxOffset + global_idx_offset,
					.m_start_vertex = (int)pcmd->VtxOffset + global_vtx_offset,
					.m_start_instance = 0u
					});
			}

			global_idx_offset += cmd_list->IdxBuffer.Size;
			global_vtx_offset += cmd_list->VtxBuffer.Size;
		}
	}

	void imgui_manager::create_fonts_texture(graphics::device* device)
	{
		renderer_backend& backend = renderer_backend::get_instance();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		int bytes_per_pixel = 0u;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
		uint32 num_pixels = width * height;

		// setup tex create args
		texture_desc texture_args{};
		texture_args.m_width = width;
		texture_args.m_heigth = height;
		mp_fonts_texture = backend.create_texture("tex_imgui_fonts", texture_args);

		// setup texture data
		texture_data tex_data{};
		constexpr uint32 k_num_channels = 4u;
		for (uint32 i = 0u; i < num_pixels; ++i)
		{
			tex_data.m_pixels.push_back({});
			pixel32& pixel = tex_data.m_pixels.back();

			uint32 r = pixels[(i * k_num_channels) + 0u];
			uint32 g = pixels[(i * k_num_channels) + 1u];
			uint32 b = pixels[(i * k_num_channels) + 2u];
			uint32 a = pixels[(i * k_num_channels) + 3u];

			pixel = make_pixel32(r, g, b, a);
		}

		backend.upload_texture_data(mp_fonts_texture, tex_data);
	}

	void imgui_manager::create_shaders()
	{
		shader::compile_args args{};
		args.m_entrypoint = "main";
#if INFLUX_DEBUG
		args.m_compile_debug = true;
#else
		args.m_compile_debug = false;
#endif
		args.m_target = shader::e_shader_target::_6_2;
		args.m_pbd = true;
		args.m_reflection;

		args.m_type = shader::e_shader_type::vs;
		m_vertex_shader = shader::compile_shader_source(k_vertex_shader, args);

		args.m_type = shader::e_shader_type::ps;
		m_pixel_shader = shader::compile_shader_source(k_pixel_shader, args);
	}

	void imgui_manager::create_pipeline(graphics::device* device)
	{
		// setup root signature
#pragma region root_signature
		graphics::rootsignature_desc rootsig_desc{};
		graphics::root_param_constants constants
		{
			16u, // num_dwords
			0u, // shader_reg
			0u,	// register_space
			graphics::e_shader_visibility::vertex
		};
		rootsig_desc.m_constants.push_back(constants);

		rootsig_desc.add_root_range( 
			graphics::root_param_resource_range::e_type::srv,
			1u,
			0u,
			0u,
			graphics::e_shader_visibility::pixel);

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
#pragma endregion

		// create shaders
		create_shaders();

		// setup pipeline
		graphics::pipeline_desc pipeline_desc{};
		pipeline_desc.m_vs = m_vertex_shader.m_bytecode;
		pipeline_desc.m_ps = m_pixel_shader.m_bytecode;
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
		pipeline_desc.m_blends[0u].m_enabled = true;
		pipeline_desc.m_blends[0u].m_src		= graphics::e_blend::src_alpha;
		pipeline_desc.m_blends[0u].m_dest		= graphics::e_blend::inv_src_alpha;
		pipeline_desc.m_blends[0u].m_op			= graphics::e_blendop::add;
		pipeline_desc.m_blends[0u].m_srcalpha	= graphics::e_blend::one;
		pipeline_desc.m_blends[0u].m_destalpha	= graphics::e_blend::inv_src_alpha;
		pipeline_desc.m_blends[0u].m_op_alpha	= graphics::e_blendop::add;
		pipeline_desc.m_blends[0u].m_write_mask = 15u; // all

		pipeline_desc;

		// rasterizer
		pipeline_desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
		pipeline_desc.m_rasterizer.m_fillmode = graphics::e_fill_mode::solid;
		pipeline_desc.m_rasterizer.m_front_ccw = false;
		pipeline_desc.m_rasterizer.m_depth_bias = 0;
		pipeline_desc.m_rasterizer.m_depth_bias_clamp = 0.0f;
		pipeline_desc.m_rasterizer.m_slope_depth_bias = 0.0f;
		pipeline_desc.m_rasterizer.m_depth_clip_enable = true;
		pipeline_desc.m_rasterizer.m_multisample = false;
		pipeline_desc.m_rasterizer.m_antialiased_line = false;
		pipeline_desc.m_rasterizer.m_forced_samplecount = 0u;
		pipeline_desc.m_rasterizer.m_conservative = false;

		// depth stencil
		pipeline_desc.m_depth_stencil.m_depth_enable = false;
		pipeline_desc.m_depth_stencil.m_stencil_enable = false;

		mp_pipeline = device->create_pipeline(mp_rootsig, pipeline_desc);
	}

	void imgui_manager::update_buffers(ImDrawData* draw_data)
	{
		const uint32 num_vertices = (mp_vertexbuffer == nullptr) ?
			0u : (uint32)(mp_vertexbuffer->get_bytesize() / sizeof(ImDrawVert));

		const uint32 num_indices = mp_indexbuffer == nullptr ?
			0u : (uint32)(mp_indexbuffer->get_bytesize() / sizeof(ImDrawIdx));

		// recreate resources if necessary
		if (num_vertices < (uint32)draw_data->TotalVtxCount)
		{
			if (mp_vertexbuffer) mp_vertexbuffer->release(mp_device);
			const uint32 new_num_vertices = draw_data->TotalVtxCount + 5000u;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_vertices * sizeof(ImDrawVert);
			desc.m_bytestride = sizeof(ImDrawVert);
			desc.m_init_state = graphics::e_resource_state::gen_read;

			mp_vertexbuffer = mp_device->create_resource(desc, heap_desc);
		}
		if (num_indices < (uint32)draw_data->TotalIdxCount)
		{
			if (mp_indexbuffer) mp_indexbuffer->release(mp_device);
			const uint32 new_num_indices = draw_data->TotalIdxCount + 10000;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_indices * sizeof(ImDrawIdx);
			desc.m_format = graphics::e_format::u16;
			desc.m_init_state = graphics::e_resource_state::gen_read;

			mp_indexbuffer = mp_device->create_resource(desc, heap_desc);
		}

		// map buffer data
		mp_vertexbuffer->map([&draw_data](void* dest)
		{
			ImDrawVert* vtx_dst = (ImDrawVert*)dest;
			for (int n = 0u; n < draw_data->CmdListsCount; ++n)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				vtx_dst += cmd_list->VtxBuffer.Size;
			}
		});
		mp_indexbuffer->map([&draw_data](void* dest)
		{
			ImDrawIdx* idx_dst = (ImDrawIdx*)dest;
			for (int n = 0u; n < draw_data->CmdListsCount; ++n)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				idx_dst += cmd_list->IdxBuffer.Size;
			}
		});
	}
}