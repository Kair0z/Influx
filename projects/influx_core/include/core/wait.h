#pragma once

#include "core/function.h"
#include "core/time.h"

namespace influx
{
	struct wait_args final
	{
		wait_args() = default;
		wait_args(float* out_seconds_waited, function<void()> wait_tick = {}) : mp_out_ms_waited{ out_seconds_waited }, m_wait_tick{ wait_tick } {}

		float* mp_out_ms_waited = nullptr;
		float m_max_ms = FLT_MAX;
		function<void()> m_wait_tick = {};
	};

	inline void wait(const function<bool()>& pred, const wait_args& args = {})
	{
		time::point wait_start = time::get_now();
		time::point wait_end = time::get_now();
		while (pred() == false && time::get_ms_since<float>(wait_start) < args.m_max_ms)
		{
			args.m_wait_tick();
		}
	}
}