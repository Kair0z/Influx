#include "renderer_pch.h"
#include "renderer_imgui.h"

#include "imgui/imgui.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/resources/resource_manager.h"
#include "influx_renderer/common.h"

// influx::rendergraph
#include "rendergraph.h"

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
		: mp_indexbuffer{ renderer_backend::get_instance() }
		, mp_vertexbuffer{ renderer_backend::get_instance() }
		, mp_device{device}
	{
		create_fonts_texture(device);
		create_pipeline(device);
	}

	void imgui_manager::build_rendergraph(rendergraph::rgpass_builder& builder, const target& target, const ImDrawData& drawdata)
	{
		// register write
		rendergraph::rgaccess access{};
		access.m_load = rendergraph::e_rg_load::preserve;
		access.m_store = rendergraph::e_rg_store::preserve;
		builder.write_rendertarget(target.get_name(), access);

		// register reads
		// each texture dependency imgui wants, we should import into the graph as well!
		auto texture_reads = imgui_manager::get_texture_dependencies(&drawdata);
		for (const auto& texture : texture_reads)
		{
			builder.read_texture((graphics::resource*)texture->get_tex_resource());
		}
		builder.set_viewport(target.get_width(), target.get_height());
	}

	void imgui_manager::render(graphics::commandlist* commandlist, const ImDrawData& draw, const target& target)
	{
		render(commandlist, vector<ImDrawData const*>{ &draw }, vector<renderer::target const*>{ &target });
	}

	void imgui_manager::render(graphics::commandlist* commandlist, const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
	{
		if (draws.size() <= 0u) 
			return;
		if (targets.size() <= 0u)
			return;

		renderer_backend& backend = renderer_backend::get_instance();
		rhi_device& device = backend.get_device();
		descriptor_manager& descriptor_manager = *backend.get_descriptor_manager();

		// execute for each draw
		for (uint32 i = 0u; i < draws.size(); ++i)
		{
			const ImDrawData& draw	= *draws[i];
			const target& target	= *targets[i];

			// update vertex / index buffers
			update_buffers(draws);

			graphics::resource* indexbuffer = mp_indexbuffer.get_cpu();
			graphics::resource* vertexbuffer = mp_vertexbuffer.get_cpu();
			if (vertexbuffer && indexbuffer)
			{
				commandlist->set_vertexbuffer(vertexbuffer);
				commandlist->set_indexbuffer(indexbuffer);
			}

			commandlist->set_primitive_topology(graphics::e_primitive_topology::trilist);
			commandlist->set_pipeline(mp_pipeline);
			commandlist->set_rootsignature(mp_rootsig);

			// Avoid rendering when minimized
			if (draw.DisplaySize.x <= 0.0f || draw.DisplaySize.y <= 0.0f)
				return;
			if (draw.CmdListsCount <= 0)
				return;

			// set viewport
			const math::vectorf2& target_dim = { target.get_width(), target.get_height() };
			graphics::viewport viewport{};
			viewport.m_width = target_dim.x;
			viewport.m_height = target_dim.y;
			viewport.m_depth_min = 0.0f;
			viewport.m_depth_max = 1.0f;
			commandlist->set_viewport(viewport);

			// set constants
			struct vertex_const_buffer
			{
				float  m_mvp[4][4];
			};
			vertex_const_buffer vertex_constant_buffer;
			{
				float L = draw.DisplayPos.x;
				float R = draw.DisplayPos.x + draw.DisplaySize.x;
				float T = draw.DisplayPos.y;
				float B = draw.DisplayPos.y + draw.DisplaySize.y;
				float mvp[4][4] =
				{
					{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
					{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
					{ 0.0f,         0.0f,           0.5f,       0.0f },
					{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
				};

				const bool transpose = true;
				if (transpose)
				{
					std::swap(mvp[0][1], mvp[1][0]);
					std::swap(mvp[0][2], mvp[2][0]);
					std::swap(mvp[0][3], mvp[3][0]);
					std::swap(mvp[2][1], mvp[1][2]);
					std::swap(mvp[3][1], mvp[1][3]);
					std::swap(mvp[3][2], mvp[2][3]);
				}
				memcpy(&vertex_constant_buffer.m_mvp, mvp, sizeof(mvp));
			}
			commandlist->set_root_constants(0u, 16u, &vertex_constant_buffer);

			// stage the font srv onto the gpu heap
			graphics::descriptor_range font_gpu_range = descriptor_manager.stage(device, mp_fonts_texture->get_srv().get());
			graphics::descriptor_range tex_gpu_range = font_gpu_range;

			// setup draw
			// (Because we merged all buffers into a single one, we maintain our own offset into them)
			int global_vtx_offset = (int)m_per_draw_vertex_offsets[i];
			int global_idx_offset = (int)m_per_draw_index_offsets[i];
			ImVec2 clip_off = draw.DisplayPos;
			for (int n = 0; n < draw.CmdListsCount; ++n)
			{
				const ImDrawList* cmd_list = draw.CmdLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
				{
					const ImDrawCmd& command = cmd_list->CmdBuffer[cmd_i];

					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_min(command.ClipRect.x - clip_off.x, command.ClipRect.y - clip_off.y);
					ImVec2 clip_max(command.ClipRect.z - clip_off.x, command.ClipRect.w - clip_off.y);
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;

					graphics::rect rect
					{
						.m_left = (uint32)clip_min.x,
						.m_top = (uint32)clip_min.y,
						.m_right = (uint32)clip_max.x,
						.m_bottom = (uint32)clip_max.y,
					};
					commandlist->set_scissor_rect(rect);

					// if this command has a bound TexID (descriptor*/void*),
					// we should stage the texture (allocate gpu descriptor)
					// and bind that range to the commandlist
					const bool command_has_texture = command.GetTexID() != 0u;
					if (command_has_texture)
					{
						imgui_texid_provider* tex_provider = reinterpret_cast<imgui_texid_provider*>(command.GetTexID());
						graphics::descriptor_handle cpu_descriptor = tex_provider->get_tex_descriptor();
						graphics::resource* resource = reinterpret_cast<graphics::resource*>(tex_provider->get_tex_resource());
						
						// stage the descriptor
						tex_gpu_range = descriptor_manager.stage(device, cpu_descriptor);

						// ensure transition resource to readable
						resource->transition(commandlist, graphics::e_resource_state::ps_srv);
					}
					else
					{
						tex_gpu_range = font_gpu_range;
					}

					commandlist->set_descriptor_range(tex_gpu_range, 1u);
					commandlist->draw_indexed({
						.m_num_indexes_per_instance = command.ElemCount,
						.m_num_instances = 1u,
						.m_start_index = command.IdxOffset + global_idx_offset,
						.m_start_vertex = (int)command.VtxOffset + global_vtx_offset,
						.m_start_instance = 0u
						});
				}

				global_idx_offset += cmd_list->IdxBuffer.Size;
				global_vtx_offset += cmd_list->VtxBuffer.Size;
			}
		}
	}

	vector<imgui_texid_provider*> imgui_manager::get_texture_dependencies(const vector<ImDrawData const*>& draws)
	{
		vector<imgui_texid_provider*> result{};
		for (const auto& group : draws)
		{
			for (const auto& dependencies : get_texture_dependencies(group))
			{
				result.push_back(dependencies);
			}
		}
		return result;
	}

	vector<imgui_texid_provider*> imgui_manager::get_texture_dependencies(ImDrawData const* draw)
	{
		vector<imgui_texid_provider*> result{};
		for (int n = 0; n < draw->CmdListsCount; ++n)
		{
			const ImDrawList* cmd_list = draw->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
			{
				const ImDrawCmd& command = cmd_list->CmdBuffer[cmd_i];
				const bool command_has_texture = command.GetTexID() != 0u;
				if (command_has_texture)
				{
					imgui_texid_provider* tex_provider = reinterpret_cast<imgui_texid_provider*>(command.GetTexID());
					result.push_back(tex_provider);
				}
			}
		}
		return result;
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

		// setup texture data
		texture_data tex_data{}; tex_data.m_width = width;
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

		const tex_id id = make_id("imgui_font");
		texture_resource& tex_resource = backend.get_resource_manager().load<e_resource_type::texture>(id, tex_data, false);
		mp_fonts_texture = tex_resource.m_resource;
	}

	result<> imgui_manager::create_shaders()
	{
		shader::compile_args args{};
		shader::shader_signature signature{};

		signature.m_entrypoint = "main";
		signature.m_filename = "imgui_shaders";
#if INFLUX_DEBUG
		args.set_debug_level(true);
#else
		args.set_debug_level(false);
#endif
		args.m_target = shader::e_shader_target::_6_6;
		args.m_pbd_enabled = true;
		args.m_reflection_enabled = true;

		signature.m_type = shader::e_shader_type::vs;
		auto res = shader::compile_shader_in_source(k_vertex_shader, signature, args);
		if (res.is_fail())
		{
			return result<>::make_error("error: failed compiling vertex shader!");
		}

		m_vertex_shader = res.get();

		signature.m_type = shader::e_shader_type::ps;
		res = shader::compile_shader_in_source(k_pixel_shader, signature, args);
		if (res.is_fail())
		{
			return result<>::make_error("error: failed compiling pixel shader!");
		}
		
		m_pixel_shader = res.get();

		return {};
	}

	result<> imgui_manager::create_pipeline(graphics::device* device)
	{
		using result_type = result<>;
		if (device == nullptr)
			return result_type::make_error("error: invalid device!");

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
		auto res = create_shaders();
		influx_assert(res.is_success());

		// setup pipeline
		graphics::graphics_pipeline_desc pipeline_desc{};
		pipeline_desc.m_shaders.set(graphics::e_graphics_shader_slots::vs, m_vertex_shader.m_bytecode);
		pipeline_desc.m_shaders.set(graphics::e_graphics_shader_slots::ps, m_pixel_shader.m_bytecode);
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
		pipeline_desc.m_blend_alpha_to_coverage_enabled = false;
		pipeline_desc.m_blends[0u].m_enabled	= true;
		pipeline_desc.m_blends[0u].m_src		= graphics::e_blend::src_alpha;
		pipeline_desc.m_blends[0u].m_dest		= graphics::e_blend::inv_src_alpha;
		pipeline_desc.m_blends[0u].m_op			= graphics::e_blendop::add;
		pipeline_desc.m_blends[0u].m_srcalpha	= graphics::e_blend::one;
		pipeline_desc.m_blends[0u].m_destalpha	= graphics::e_blend::inv_src_alpha;
		pipeline_desc.m_blends[0u].m_op_alpha	= graphics::e_blendop::add;
		pipeline_desc.m_blends[0u].m_write_mask = 15u; // all

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

		mp_pipeline = device->create_graphics_pipeline(mp_rootsig, pipeline_desc);
		if (mp_pipeline == nullptr)
			return result_type::make_error("failed creating graphics pipeline!");

		return {};
	}

	void imgui_manager::update_buffers(const vector<ImDrawData const*>& draws)
	{
		uint32 total_num_vertices = 0u;
		uint32 total_num_indices = 0u;
		m_per_draw_vertex_offsets.clear();
		m_per_draw_index_offsets.clear();

		for (uint32 i = 0u; i < draws.size(); ++i)
		{
			const ImDrawData& draw = *draws[i];
			if (draw.TotalVtxCount <= 0) return;

			m_per_draw_vertex_offsets.push_back(total_num_vertices);
			m_per_draw_index_offsets.push_back(total_num_indices);

			total_num_vertices += draw.TotalVtxCount;
			total_num_indices += draw.TotalIdxCount;
		}
		
		graphics::resource*& vertexbuffer = mp_vertexbuffer.get_cpu();
		graphics::resource*& indexbuffer = mp_indexbuffer.get_cpu();

		const uint32 current_num_vertices = (vertexbuffer == nullptr) ?
			0u : (uint32)(vertexbuffer->get_bytesize() / sizeof(ImDrawVert));

		const uint32 current_num_indices = indexbuffer == nullptr ?
			0u : (uint32)(indexbuffer->get_bytesize() / sizeof(ImDrawIdx));

		// recreate resources if necessary
		if (current_num_vertices < total_num_vertices)
		{
			if (vertexbuffer) vertexbuffer->release(mp_device);
			const uint32 new_num_vertices = total_num_vertices + 5000u;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_vertices * sizeof(ImDrawVert);
			desc.m_bytestride = sizeof(ImDrawVert);
			desc.m_init_state = graphics::e_resource_state::gen_read;
			vertexbuffer = mp_device->create_resource(desc, heap_desc);
		}
		if (current_num_indices < total_num_indices)
		{
			if (indexbuffer) indexbuffer->release(mp_device);
			const uint32 new_num_indices = total_num_indices + 10000;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_indices * sizeof(ImDrawIdx);
			desc.m_format = graphics::e_format::u16;
			desc.m_init_state = graphics::e_resource_state::gen_read;
			indexbuffer = mp_device->create_resource(desc, heap_desc);
		}

		// map buffer data
		if (vertexbuffer)
		{
			vertexbuffer->map([&draws, this](void* dest)
			{
				for (uint32 i = 0u; i < draws.size(); ++i)
				{
					ImDrawVert* vtx_dst = (ImDrawVert*)dest + m_per_draw_vertex_offsets[i];
					for (int n = 0u; n < draws[i]->CmdListsCount; ++n)
					{
						const ImDrawList* cmd_list = draws[i]->CmdLists[n];
						memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						vtx_dst += cmd_list->VtxBuffer.Size;
					}
				}
			});
		}
		if (indexbuffer)
		{
			indexbuffer->map([&draws, this](void* dest)
			{
				for (uint32 i = 0u; i < draws.size(); ++i)
				{
					ImDrawIdx* idx_dst = (ImDrawIdx*)dest + m_per_draw_index_offsets[i];
					for (int n = 0u; n < draws[i]->CmdListsCount; ++n)
					{
						const ImDrawList* cmd_list = draws[i]->CmdLists[n];
						memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
						idx_dst += cmd_list->IdxBuffer.Size;
					}
				}
			});
		}
	}
}