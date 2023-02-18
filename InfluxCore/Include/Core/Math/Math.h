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
		constexpr float k_E			= 2.718281828459045f;
		constexpr float k_PI		= 3.1415926535897932384626433832795f;
		constexpr double k_dPI		= 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
		constexpr float k_PIDouble	= 6.283185307179586476925286766559f;
		constexpr float k_PIonTwo	= 1.5707963267948966192313216916398f;
		constexpr float k_PIonFour	= 0.78539816339744830961566084581988f;
		constexpr float k_PIonSix	= 0.52359877559829887307710723054658f;
	}

	namespace Math
	{
		template <typename _T, typename _F>
		constexpr inline _T Round(const _F& fValue)
		{
			return static_cast<_T>(std::round(fValue));
		}

		template <typename _T>
		inline _T Cos(const _T& value)
		{
			return static_cast<_T>(std::cos(value));
		}

		template <typename _T>
		inline _T Sin(const _T& value)
		{
			return static_cast<_T>(std::sin(value));
		}

		template <typename _T>
		constexpr inline _T Abs(const _T& value)
		{
			return static_cast<_T>(std::abs(value));
		}

		template <typename _T>
		constexpr inline _T Lerp(const _T& t, const _T& min, const _T& max)
		{
			return min + ((max - min) * t);
		}

		template <typename _T>
		constexpr inline _T Remap(const _T& value, const _T& oldMin, const _T& oldMax, const _T& newMin, const _T& newMax)
		{
			if (oldMax - oldMin == (_T)0) return (_T)0;
			return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
		}

		template <typename _T>
		constexpr inline _T PingPong(const _T& t, const _T& range)
		{
			_T range2 = range * 2;
			_T mod = std::fmod(t, range2);
			if (mod < range)
			{
				return mod;
			}
			else
			{
				return range2 - mod;
			}
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
			if (a == b && chooseAWhenEqual) return a;
			else return b;

			if (a < b) return b;
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
			return degrees * (_T)(k_PI / 180);
		}
	}
}
#endif


