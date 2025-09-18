#pragma once

#if INFLUX_PLATFORM_WINDOWS
#include <immintrin.h>
// #include <experimental/simd>
#endif

namespace influx
{
	using uint32 = unsigned int;

	template <typename _t, uint32 _nv>
	class simd final
	{
		
	};
}