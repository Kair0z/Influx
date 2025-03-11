#pragma once

// influx::core
#include "core/time.h"

namespace influx::engine
{
	struct frame_time final
	{
	public:
		float get_delta_seconds() const
		{
			return m_delta_seconds;
		}

		float get_time_seconds() const
		{
			return m_time_seconds;
		}

	private:
		float m_delta_seconds;
		float m_time_seconds;

		influx::time::point m_first_tick;
		influx::time::point m_last_tick;
		bool m_is_first_tick = true;
		bool m_is_fixed = false;

	public:
		inline void tick()
		{
			if (m_is_first_tick)
			{
				m_first_tick = influx::time::get_now();
				m_last_tick = m_first_tick;
				m_is_first_tick = false;
			}

			if (m_is_fixed)
			{
				m_delta_seconds = 1.0f / 60.0f;
			}
			else
			{
				const float delta_seconds = influx::time::get_ms_since<float>(m_last_tick) * 0.001f;
				m_delta_seconds = delta_seconds;
				m_last_tick = influx::time::get_now();
			}

			m_time_seconds += m_delta_seconds;
		}

		void set_fixed(bool is_fixed)
		{
			m_is_fixed = is_fixed;
		}
	};

	struct update_context final
	{
		frame_time m_frametime;
	};
}