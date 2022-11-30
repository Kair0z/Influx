#pragma once

#ifndef _CORE_MATH_H_
#define _CORE_MATH_H_

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "Vector.h"

#include <algorithm>
#include <type_traits>
#include <limits>

namespace Influx
{
	namespace Math
	{
		template <typename _T, typename _F>
		constexpr inline _T Round(const _F& fValue)
		{
			return static_cast<_T>(std::round(fValue));
		}

		template <typename _T>
		constexpr inline _T Abs(const _T& value)
		{
			return std::abs(value);
		}

		template <typename _T>
		constexpr inline _T Lerp(const _T& t, const _T& min, const _T& max)
		{
			return (max - min) * t;
		}

		template <typename _T>
		constexpr inline _T IsZero(const _T& value)
		{
#define epsilon 0.00001f;
			return Abs(value) < epsilon;
		}

#pragma region MinMax
		template <typename _T>
		constexpr inline _T Max(const _T& a, const _T& b, const bool chooseAWhenEqual = true)
		{
			if constexpr (a == b && chooseAWhenEqual) return a;
			else return b;

			if constexpr (a < b) return b;
			else return b;
		}

		template <typename _T>
		constexpr inline _T Min(const _T& a, const _T& b, const bool chooseAWhenEqual = true)
		{
			if constexpr (a == b && chooseAWhenEqual) return a;
			else return b;

			if constexpr (a < b) return a;
			else return b;
		}

		template <typename _T>
		constexpr inline _T Max(std::initializer_list<_T> list)
		{
			return std::max(list);
		}

		template <typename _T>
		constexpr inline _T Min(std::initializer_list<_T> list)
		{
			return std::min(list);
		}

		namespace Internal
		{
			template <typename _T>
			constexpr _T const& DoMax(_T const& v) { return v; }

			template <typename _T, class... _R>
			constexpr _T const& DoMax(_T const& v0, _T const& v1, _R const&... rest)
			{
				return DoMax(v0 < v1 ? v1 : v0, rest...);
			}

			template <typename _T>
			constexpr _T const& DoMin(_T const& v) { return v; }

			template <typename _T, class... _R>
			constexpr _T const& DoMin(_T const& v0, _T const& v1, _R const&... rest)
			{
				return DoMin(v0 < v1 ? v0 : v1, rest...);
			}
		}
		
		template <typename _T, class... _R>
		inline constexpr _T const& Max(_T const& a, _R const&... rest)
		{
			return Internal::DoMax(a, rest...);
		}

		template <typename _T, class... _R>
		inline constexpr _T const& Min(_T const& a, _R const&... rest)
		{
			return Internal::DoMin(a, rest...);
		}

#pragma endregion

		template <typename _T>
		constexpr inline _T MiddleOfThree(const _T& a, const _T& b, const _T& c, const bool chooseMinWhenEqual = true)
		{
			constexpr _T min = Min(a, b, c);
			constexpr _T max = Max(a, b, c);

			if constexpr (a != min && a != max) return a;
			if constexpr (b != min && b != max) return b;
			if constexpr (c != min && c != max) return c;

			// There's an equality...
			return min;
		}

		template <typename _T>
		constexpr _T Clamp(const _T& x, const _T& lo, const _T& hi) noexcept
		{
			return x < lo ? lo : (x > hi ? hi : x);
		}

		template <typename _T, typename = std::enable_if_t<std::is_unsigned_v<std::decay_t<_T>>>>
		constexpr _T ClampBitwise(_T x, _T lo, _T hi) noexcept
		{
			return (x & hi) | lo;
		}

		template <typename _T>
		constexpr inline _T DegreesToRadians(_T degrees)
		{
			return degrees * (_T)(3.1415 / 180);
		}

		constexpr inline float PIf = 3.1415f;
	}
}
#endif


