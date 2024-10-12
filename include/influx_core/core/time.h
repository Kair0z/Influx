#pragma once

#include "core/cast.h"
#include "core/function.h"

// STL:
#include <chrono>

namespace influx::time
{
	using point = std::chrono::steady_clock::time_point;

	static point get_now() noexcept
	{
		return std::chrono::steady_clock::now();
	}

	template <typename _t>
	static _t get_ms_between(const point& end, const point& start) noexcept
	{
		return static_cast<_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}

	template <typename _t>
	static _t get_ms_since(const point& point)
	{
		return get_ms_between<_t>(get_now(), point);
	}

	template <typename _t>
	static _t measure_ms(const function<void()>& function)
	{
		point before = get_now();
		function();
		return get_ms_between<_t>(get_now(), before);
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
}