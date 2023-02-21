#pragma once

#ifndef __CORE_TIME_H_
#define __CORE_TIME_H_

// Dependencies:
#ifdef __USECORE_
#undef __USECORE_
#endif

#define __USECORE_ 1

#if		__USECORE_
#include "Core/Cast.h"
#else
template <typename _Dest, typename _T>
inline _Dest* StaticCast(_T* p)
{
	return static_cast<_Dest*>(p);
}

template <typename _Dest, typename _T>
inline const _Dest* StaticCast(const _T* p)
{
	return static_cast<const _Dest*>(p);
}
#endif
#undef __USECORE_

// STL:
#include <chrono>

namespace Influx
{
	class Time final
	{
	public:
		using TimePoint = std::chrono::system_clock::time_point;

		inline static TimePoint Now() noexcept
		{
			return std::chrono::system_clock::now();
		}

		template <typename _T>
		inline static _T MsBetween(const TimePoint& end, const TimePoint& start) noexcept
		{
			return static_cast<_T>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
		}
	};
}

#endif


