#pragma once

#include "influx_async.h"
#include "core/time.h"

namespace influx::async
{
	struct task_data final
	{
		task_data() = default;
		task_data(const task_create_args& args)
			: m_args{ args }
		{
		}

		void set_state(e_task_state new_state)
		{
			switch (new_state)
			{
			case e_task_state::allocated:
				m_time_allocated = time::get_now();
				break;

			case e_task_state::pending:
				m_time_queued = time::get_now();
				break;

			case e_task_state::running:
				m_time_started = time::get_now();
				break;

			case e_task_state::finished:
				m_time_finished = time::get_now();
				break;
			}

			m_state = new_state;
		}

		void reset(e_task_state new_state)
		{
			m_time_allocated = m_time_queued = m_time_started = m_time_finished = time::get_now();
			m_refcount = 0u;
			m_stats = task_stats{};

			set_state(new_state);
		}

		inline bool is_finished() const
		{
			return m_state == e_task_state::finished;
		}

		e_task_state m_state{};
		task_create_args m_args{};
		task_stats m_stats{};

		std::atomic_uint32_t m_refcount = 0u;

		time::point m_time_allocated = time::get_now();
		time::point m_time_queued = time::get_now();
		time::point m_time_started = time::get_now();
		time::point m_time_finished = time::get_now();
	};
}
