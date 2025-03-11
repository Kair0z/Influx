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

	class wait_handle final
	{
	public:
		inline wait_handle()
		{
			reset();
		}

		inline wait_handle(const wait_args& args)
			: m_ms_waited{}
			, m_ms_waited_accum{}
			, m_args{args}
		{
			
		}

		inline void add_ms_waited(float ms_waited)
		{
			m_ms_waited = ms_waited;
			m_ms_waited_accum += m_ms_waited;
		}

		inline float get_ms_waited() const
		{
			return m_ms_waited_accum;
		}

		inline float get_recent_ms_waited() const
		{
			return m_ms_waited;
		}

		inline float get_ms_max() const
		{
			return m_args.m_max_ms;
		}

		inline const function<void()>& get_wait_tick()
		{
			return m_args.m_wait_tick;
		}

		inline void reset()
		{
			m_ms_waited = m_ms_waited_accum = 0;
		}

		inline void set_max_ms_waited(float ms_max)
		{
			m_args.m_max_ms = ms_max;
		}

	private:
		float m_ms_waited{};
		float m_ms_waited_accum{};
		wait_args m_args{};
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