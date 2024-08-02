#include "imgui_pch.h"
#include "influx_imgui.h"

#include "core/singleton.h"

#include "influx_graphics/resource.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/descriptorheap.h"
#include "influx_graphics/device.h"

namespace influx::imgui
{
	struct render_buffers
	{
		graphics::resource* mp_indexbuffer;
		graphics::resource* mp_vertexbuffer;
	};

	struct global_state : public singleton<global_state>
	{
		render_buffers m_renderbuffers;
		graphics::device* mp_device;
		graphics::command_queue* mp_commandqueue;
		graphics::command_list* mp_commandlist;
		graphics::command_allocator* mp_allocator;
		graphics::rootsignature* mp_rootsig;
		graphics::pipeline* mp_pipeline;
	};

	inline graphics::device*& get_device()
	{
		return global_state::get_instance().mp_device;
	}

	inline graphics::command_queue*& get_queue()
	{
		return global_state::get_instance().mp_commandqueue;
	}

	inline graphics::command_list*& get_commandlist()
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

	inline graphics::pipeline*& get_pipeline()
	{
		return global_state::get_instance().mp_pipeline;
	}

	inline graphics::rootsignature*& get_rootsig()
	{
		return global_state::get_instance().mp_rootsig;
	}

	inline void update_renderbuffers(ImDrawData* draw_data, render_buffers& buffers)
	{
		const uint32 num_vertices = (buffers.mp_vertexbuffer == nullptr) ? 
			0u : buffers.mp_vertexbuffer->get_bytesize() / sizeof(ImDrawVert);

		const uint32 num_indices = buffers.mp_indexbuffer == nullptr ? 
			0u : buffers.mp_indexbuffer->get_bytesize() / sizeof(ImDrawIdx);

		// recreate resources if necessary
		if (num_vertices < draw_data->TotalVtxCount)
		{
			delete buffers.mp_vertexbuffer;
			const uint32 new_num_vertices = draw_data->TotalVtxCount + 5000u;

			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;
			graphics::buffer_desc desc{};
			desc.m_bytesize = new_num_vertices * sizeof(ImDrawVert);

			buffers.mp_vertexbuffer = get_device()->create_resource(desc, heap_desc);
		}
		if (num_indices < draw_data->TotalIdxCount)
		{
			delete buffers.mp_indexbuffer;
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

	bool initialize()
	{
		// create the device
		get_device() = graphics::device::create(graphics::e_api_type::dx12);

		// create the command queue
		graphics::command_queue_desc queue_desc{};
		queue_desc.m_type = graphics::e_command_queue_type::graphics;
		get_queue() = get_device()->create_command_queue(queue_desc);

		// create command list & allocator
		get_allocator() = get_device()->create_graphics_allocator();
		get_commandlist() = get_device()->create_graphics_command_list(get_allocator());

		return true;
	}

	bool shutdown()
	{
		delete get_device();
	}

	void render(ImDrawData* draw_data)
	{
		// Avoid rendering when minimized
		if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			return;

		graphics::viewport viewport{};

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

		get_commandlist()->start(get_allocator());

		// setup state
		get_commandlist()->set_vertexbuffer(get_buffers().mp_vertexbuffer);
		get_commandlist()->set_indexbuffer(get_buffers().mp_indexbuffer);
		get_commandlist()->set(viewport);
		get_commandlist()->set(graphics::e_primitive_topology::trilist);
		get_commandlist()->set(get_pipeline());
		get_commandlist()->set(get_rootsig());
		get_commandlist()->set_constants(0u, 16u, &vertex_constant_buffer);

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
					.m_left = clip_min.x,
					.m_top = clip_max.y,
					.m_right = clip_max.x,
					.m_bottom = clip_min.y,
				};
				get_commandlist()->set(rect);

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

