#include "imgui_pch.h"
#include "influx_imgui.h"

// imgui dependency (duh)
#include "imgui/imgui.h"

// influx::core
#include "core/singleton.h"

// influx::graphics
#include "influx_graphics/resource.h"
#include "influx_graphics/queue.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/device.h"

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

namespace influx::imgui
{
	struct render_buffers
	{
		graphics::resource* mp_indexbuffer;
		graphics::resource* mp_vertexbuffer;
	};

	struct texture
	{
		graphics::resource* mp_upload;
		graphics::resource* mp_resource;
		graphics::descriptor_handle m_srv_cpu;
	};

	struct global_state : public singleton<global_state>
	{
		render_buffers m_renderbuffers;
		texture m_fonts_texture;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;

		graphics::device* mp_device;
		graphics::queue* mp_commandqueue;
		graphics::commandlist* mp_commandlist;
		graphics::command_allocator* mp_allocator;
		graphics::rootsignature* mp_rootsig;
		graphics::graphics_pipeline* mp_pipeline;
		graphics::descriptor_heap* mp_srv_heap;

		graphics::fence* mp_fence;
	};

	inline graphics::device*& get_device()
	{
		return global_state::get_instance().mp_device;
	}

	inline graphics::queue*& get_queue()
	{
		return global_state::get_instance().mp_commandqueue;
	}

	inline graphics::commandlist*& get_commandlist()
	{
		return global_state::get_instance().mp_commandlist;
	}

	inline graphics::command_allocator*& get_allocator()
	{
		return global_state::get_instance().mp_allocator;
	}

	inline render_buffers& get_buffers()
	{
		return global_state::get_instance().m_renderbuffers;
	}

	inline texture& get_texfonts()
	{
		return global_state::get_instance().m_fonts_texture;
	}

	inline graphics::graphics_pipeline*& get_pipeline()
	{
		return global_state::get_instance().mp_pipeline;
	}

	inline graphics::rootsignature*& get_rootsig()
	{
		return global_state::get_instance().mp_rootsig;
	}

	inline graphics::fence*& get_fence()
	{
		return global_state::get_instance().mp_fence;
	}

	inline graphics::descriptor_heap*& get_srv_heap()
	{
		return global_state::get_instance().mp_srv_heap;
	}

	inline shader::compile_output& get_vertex_shader()
	{
		return global_state::get_instance().m_vertex_shader;
	}

	inline shader::compile_output& get_pixel_shader()
	{
		return global_state::get_instance().m_pixel_shader;
	}

	inline void update_renderbuffers(ImDrawData* draw_data, render_buffers& buffers)
	{
		const uint32 num_vertices = (buffers.mp_vertexbuffer == nullptr) ?
			0u : (uint32)(buffers.mp_vertexbuffer->get_bytesize() / sizeof(ImDrawVert));

		const uint32 num_indices = buffers.mp_indexbuffer == nullptr ?
			0u : (uint32)(buffers.mp_indexbuffer->get_bytesize() / sizeof(ImDrawIdx));

		// recreate resources if necessary
		if (num_vertices < (uint32)draw_data->TotalVtxCount)
		{
			get_device()->release(buffers.mp_vertexbuffer);
			const uint32 new_num_vertices = draw_data->TotalVtxCount + 5000u;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_vertices * sizeof(ImDrawVert);

			buffers.mp_vertexbuffer = get_device()->create_resource(desc, heap_desc);
		}
		if (num_indices < (uint32)draw_data->TotalIdxCount)
		{
			get_device()->release(buffers.mp_indexbuffer);
			const uint32 new_num_indices = draw_data->TotalIdxCount + 10000;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_indices * sizeof(ImDrawIdx);

			buffers.mp_indexbuffer = get_device()->create_resource(desc, heap_desc);
		}

		// map buffer data
		buffers.mp_vertexbuffer->map([&draw_data](void* dest)
		{
			ImDrawVert* vtx_dst = (ImDrawVert*)dest;
			for (int n = 0u; n < draw_data->CmdListsCount; ++n)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				vtx_dst += cmd_list->VtxBuffer.Size;
			}	
		});
		buffers.mp_indexbuffer->map([&draw_data](void* dest)
		{
			ImDrawIdx* idx_dst = (ImDrawIdx*)dest;
			for (int n = 0u; n < draw_data->CmdListsCount; ++n)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(idx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				idx_dst += cmd_list->VtxBuffer.Size;
			}
		});
	}

	inline void create_fonts_texture()
	{
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
		buffer_desc.m_init_state = graphics::e_resource_state::copy_src;

		graphics::tex2D_desc texture_desc{};
		texture_desc.m_dimensions = { tex_width, tex_height };
		texture_desc.m_format = tex_format;
		texture_desc.m_init_state = graphics::e_resource_state::copy_dst;

		fonts_tex.mp_upload = get_device()->create_resource(buffer_desc, { graphics::e_heap_type::shared });
		fonts_tex.mp_resource = get_device()->create_resource(texture_desc);

		// texture data -> upload res
		fonts_tex.mp_upload->map([tex_pitch, tex_height, pixels](void* dest)
		{
			for (uint32 y = 0; y < tex_height; ++y)
				memcpy((void*) ((uintptr_t) dest + y * tex_pitch),
					pixels + y * tex_pitch, tex_pitch);
		});

		// record transfer (upload resource -> gpu resource)
		get_commandlist()->start(get_device());
		get_commandlist()->copy_texture(
			fonts_tex.mp_upload, fonts_tex.mp_resource);
		get_commandlist()->end();

		// submit transfer
		get_queue()->submit({ get_commandlist() });
		get_queue()->queue_signal(get_fence(), 1u);

		// wait for transfer to finish on gpu
		wait_handle wait{};
		get_fence()->wait_for_value(1u, wait);

		// create srv
		get_device()->create_texture_srv(get_srv_heap()->allocate_cpu().get(), fonts_tex.mp_resource);
	}

	inline void create_shaders()
	{
		shader::compile_args args{};
		args.m_signature.m_entrypoint = "main";
#if INFLUX_DEBUG
		args.m_compile_debug = true;
#else
		args.m_compile_debug = false;
#endif
		args.m_signature.m_target = shader::e_shader_target::_6_2;
		args.m_pbd = true;
		args.m_reflection;

		args.m_signature.m_type = shader::e_shader_type::vs;
		get_vertex_shader() = shader::compile_shader(k_vertex_shader, args).get();

		args.m_signature.m_type = shader::e_shader_type::ps;
		get_pixel_shader() = shader::compile_shader(k_pixel_shader, args).get();
	}

	inline void create_pipeline()
	{
		// root signature
		graphics::rootsignature_desc rootsig_desc{};
		{
			// constants
			graphics::root_param_constants constants
			{
				16u, // num_dwords
				0u, // shader_reg
				0u,	// register_space
				graphics::e_shader_visibility::vertex
			};
			rootsig_desc.m_constants.push_back(constants);

			// descriptor table
			graphics::root_param_resource_range range
			{
				1u, // num_dwords
				graphics::root_param_resource_range::e_type::srv,
				0u, // shadder_reg
				0u	// register_space
			};
			rootsig_desc.add_root_range(
				graphics::root_param_resource_range::e_type::srv,
				1u, // num_dwords
				0u, // shadder_reg
				0u	// register_space
			);

			// static sampler
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

			get_rootsig() = get_device()->create_rootsignature(rootsig_desc);

		}
		
		// shaders
		create_shaders();

		// pipeline
		graphics::graphics_pipeline_desc pipeline_desc{};
		pipeline_desc.m_shaders.set(graphics::e_graphics_shader_slots::vs, get_vertex_shader().m_bytecode);
		pipeline_desc.m_shaders.set(graphics::e_graphics_shader_slots::ps, get_pixel_shader().m_bytecode);
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

		get_pipeline() = get_device()->create_graphics_pipeline(get_rootsig(), pipeline_desc);
	}

	bool initialize()
	{
		get_device() = graphics::device::create(graphics::e_api_type::dx12);

		graphics::queue_desc queue_desc{};
		queue_desc.m_type = graphics::e_queue_type::graphics;
		get_queue() = get_device()->create_queue(queue_desc);

		get_commandlist() = get_device()->create_graphics_commandlist();

		graphics::descriptor_heap::create_args desc_heap_args{};
		desc_heap_args.m_capacity = 1u;
		desc_heap_args.m_shader_visible = true;
		desc_heap_args.m_type = graphics::e_descriptor_heap_type::srv;
		get_srv_heap() = get_device()->create_descriptor_heap(desc_heap_args);

		create_fonts_texture();
		create_pipeline();

		return true;
	}

	bool shutdown()
	{
		delete get_device();
		return true;
	}

	void render(ImDrawData* draw_data, const target& target)
	{
		// Avoid rendering when minimized
		if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			return;

		const math::vectorf2& target_dim = target.mp_rtv->get_dimensions();

		graphics::viewport viewport{};
		viewport.m_width = target_dim.x;
		viewport.m_height = target_dim.y;

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

		// update vertex / index buffers
		update_renderbuffers(draw_data, global_state::get_instance().m_renderbuffers);

		get_commandlist()->start(get_device());

		// setup state
		get_commandlist()->set_vertexbuffer(get_buffers().mp_vertexbuffer);
		get_commandlist()->set_indexbuffer(get_buffers().mp_indexbuffer);
		get_commandlist()->set_viewport(viewport);
		get_commandlist()->set_primitive_topology(graphics::e_primitive_topology::trilist);
		get_commandlist()->set_pipeline(get_pipeline());
		get_commandlist()->set_rootsignature(get_rootsig());
		get_commandlist()->set_root_constants(0u, 16u, &vertex_constant_buffer);

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
					.m_top = (uint32)clip_max.y,
					.m_right = (uint32)clip_max.x,
					.m_bottom = (uint32)clip_min.y,
				};
				get_commandlist()->set_scissor_rect(rect);

				get_commandlist()->draw_indexed({
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

		get_commandlist()->end();
	}
}

