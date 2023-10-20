#pragma once

#ifndef __CORE_TIME_H_
#define __CORE_TIME_H_

// Dependencies:
#ifdef __USECORE_
#undef __USECORE_
#endif

#define __USECORE_ 1

#if		__USECORE_
#include "core/cast.h"
#include "core/function.h"
#undef __USECORE_
#endif

// STL:
#include <chrono>

namespace influx
{
	class time final
	{
	public:
		using point = std::chrono::system_clock::time_point;

		static point get_now() noexcept
		{
			return std::chrono::system_clock::now();
		}

		template <typename _t>
		static _t get_ms_between(const point& end, const point& start) noexcept
		{
			return static_cast<_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		}

		template <typename _t>
		static _t measure_ms(const function<void()>& function)
		{
			point before = get_now();
			function();
			return get_ms_between<_t>(get_now(), before);
		}
	};
}

#endif


