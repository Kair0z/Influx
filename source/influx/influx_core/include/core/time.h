#pragma once

// influx::core
#include "core/cast.h"
#include "core/function.h"
#include "core/basetypes.h"

// STL:
#include <chrono>

namespace influx::time
{
	using point = std::chrono::steady_clock::time_point;

	enum class e_tm : uint8
	{
		ns,
		ms,
		s,
		m,
		h
	};

	static point get_now() noexcept
	{
		return std::chrono::steady_clock::now();
	}

	template <typename _t, e_tm _m>
	static _t get_time_between(const point& end, const point& start) noexcept
	{
		_t ns = static_cast<_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
		if		constexpr (_m == e_tm::ns)	return ns;
		else if	constexpr (_m == e_tm::ms)	return static_cast<_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		else if constexpr (_m == e_tm::s)	return ns * static_cast<_t>(0.000,000,001		);
		else if constexpr (_m == e_tm::m)	return ns * static_cast<_t>(0.000,000,000,001	);
		else if constexpr (_m == e_tm::h)	return ns * static_cast<_t>(0.000,000,000,000,001);
	}

	template <typename _t>
	static _t get_ns_between(const point& end, const point& start) noexcept
	{ return get_time_between<_t, e_tm::ns>(end, start); }

	template <typename _t>
	static _t get_ms_between(const point& end, const point& start) noexcept
	{ return get_time_between<_t, e_tm::ms>(end, start); }

	template <typename _t>
	static _t get_ms_since(const point& point)
	{ return get_ms_between<_t>(get_now(), point); }

	template <typename _t>
	static _t measure_ms(const function<void()>& function)
	{
		point before = get_now();
		function();
		return get_ms_between<_t>(get_now(), before);
	}

	static float measure_msf(const function<void()>& function)
	{
		point before = get_now();
		function();
		return get_ms_between<float>(get_now(), before);
	}

	static bool is_left_sooner(const point& a, const point& b)
	{
		return a < b;
	}

	class range final
	{
	public:
		range()
		{
			reset();
		}

		range(const point& start, const point& end)
			: m_start{start}, m_end{end}
		{

		}

		float get_msf() const
		{
			return get_ms_between<float>(m_end, m_start);
		}

		void reset()
		{
			m_start = m_end = get_now();
		}

		void set_start(const point& new_start)
		{
			m_start = new_start;
		}

		void set_end(const point& new_end)
		{
			m_end = new_end;
		}

		const point& get_start() const
		{
			return m_start;
		}

		const point& get_end() const
		{
			return m_end;
		}

	private:
		point m_start;
		point m_end;
	};

	class profiler
	{
	public:
		struct result
		{
			float m_ms_average = 0.0f;
		};

		static result run(const function<void()>& func, const uint32 num_iterations)
		{
			float sum_ms = 0.0f;
			for (uint32 i = 0u; i < num_iterations; ++i)
			{
				sum_ms += measure_msf(func);
			}

			result new_result{};
			new_result.m_ms_average = sum_ms / num_iterations;
			return new_result;
		}
	};
}