#pragma once

// influx::core
#include "core/time.h"

namespace influx::engine
{
	struct frame_time final
	{
		float m_delta_seconds;
		float m_time_seconds;

		influx::time::point m_first_tick;
		influx::time::point m_last_tick;
		bool m_is_first_tick = true;

		inline void tick()
		{
			if (m_is_first_tick)
			{
				m_first_tick = influx::time::get_now();
				m_last_tick = m_first_tick;
				m_is_first_tick = false;
			}

			const float delta_seconds = influx::time::get_ms_since<float>(m_last_tick) * 0.001f;
			m_delta_seconds = delta_seconds;
			m_time_seconds += delta_seconds;

			m_last_tick = influx::time::get_now();
		}
	};
}