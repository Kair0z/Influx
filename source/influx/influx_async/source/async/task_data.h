#pragma once

#include "influx_async.h"
#include "core/time.h"

namespace influx::async
{
	struct task_data final
	{
		task_create_args		m_args{};
		std::atomic_uint32_t	m_refcount = 0u;
		e_task_state			m_state{};

#if !INFLUX_ASYNC_OMIT_STATS || 1
		task_stats	m_stats{};
		time::point m_time_allocated = time::get_now();
		time::point m_time_queued = time::get_now();
		time::point m_time_started = time::get_now();
		time::point m_time_finished = time::get_now();
#endif

		task_data() = default;
		task_data(const task_create_args& args)
			: m_args{ args }
		{
		}

		inline void update_timings(e_task_state new_state)
		{
#if !INFLUX_ASYNC_OMIT_STATS
			switch (new_state)
			{
			case e_task_state::allocated:
				m_time_allocated = time::get_now();
				break;

			case e_task_state::queued:
				m_time_queued = time::get_now();
				break;

			case e_task_state::running:
				m_time_started = time::get_now();
				break;

			case e_task_state::finished:
				m_time_finished = time::get_now();
				break;
			}
#endif
		}
		inline void set_state(e_task_state new_state)
		{
			update_timings(new_state);
			m_state = new_state;
		}

		inline void reset(e_task_state new_state)
		{
#if !INFLUX_ASYNC_OMIT_STATS
			m_time_allocated = m_time_queued = m_time_started = m_time_finished = time::get_now();
			m_stats = task_stats{};
#endif
			m_refcount = 0u;
			set_state(new_state);
		}

		inline bool is_finished() const
		{
			return m_state == e_task_state::finished;
		}
	};
}
