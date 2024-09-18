#include "graphics_pch.h"
#include "influx_graphics.h"

#include "dx12_commandbuffer.h"
#include "dx12_queue.h"
#include "dx12_fence.h"

namespace influx::graphics
{
	dx12_commandbuffer::dx12_commandbuffer(graphics::dx12_queue* queue, graphics::dx12_fence* fence)
		: m_fence{ fence }
		, m_queue{ queue }
	{
	}

	void dx12_commandbuffer::submit()
	{
		submit(m_queue);
	}

	void dx12_commandbuffer::submit(graphics::queue* queue)
	{
		influx_assert(is_finished_gpu());

		if (m_queue)
		{
			// submit work to gpu
			vector<commandlist*> lists{};
			for (const detail::command_base* command : m_commands)
			{
				
			}

			m_queue->submit_commandlists(lists);

			set_state(e_state::submitted);

			m_fence->queue_signal(m_finished_value, m_queue);

			++m_finished_value;
		}
	}

	commandbuffer::e_state dx12_commandbuffer::get_state() const
	{
		if (m_state == e_state::idle)
		{
			return e_state::idle;
		}
		else 
		{
			// in case of not idle, return a state based on the fence value
			if (m_fence->query_value() < m_finished_value)
			{
				return e_state::submitted;
			}
			else
			{
				return e_state::finished;
			}
		}
	}

	void dx12_commandbuffer::set_state(e_state new_state)
	{
		m_state = new_state;
	}
}