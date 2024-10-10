#pragma once

#include "influx_application.h"

// core includes
#include "core/singleton.h"
#include "core/container/vector.h"
#include "core/container/ringBuffer.h"
#include "core/math/matrix.h"
#include "core/math/transform.h"
#include "core/math/random.h"
#include "core/geometry/quad.h"
#include "core/geometry/geometry.h"
#include "core/time.h"
#include "core/platform/platform.h"
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"
#include "core/scope.h"
#include "core/threading/thread.h"

#include "application/constants.h"

// common
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
