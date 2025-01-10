#pragma once

#ifndef __CORE_MATH_H_
#define __CORE_MATH_H_

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "core/container/vector.h"
#include "core/math/vector.h"

#include <algorithm>
#include <type_traits>
#include <limits>

namespace influx
{
	namespace math
	{
		constexpr float k_E			= 2.718281828459045f;
		constexpr float k_PI		= 3.1415926535897932384626433832795f;
		constexpr float k_PIHalf	= k_PI / 2.0f;
		constexpr double k_dPI		= 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
		constexpr float k_PIDouble	= 6.283185307179586476925286766559f;
		constexpr float k_PIonTwo	= 1.5707963267948966192313216916398f;
		constexpr float k_PIonFour	= 0.78539816339744830961566084581988f;
		constexpr float k_PIonSix	= 0.52359877559829887307710723054658f;

		constexpr float k_epsilon = 0.00001f;
	}

	namespace math
	{
		template <typename _t, typename _F>
		constexpr inline _t round(const _F& fValue)
		{
			return static_cast<_t>(std::round(fValue));
		}

		template <typename _t, typename _f>
		constexpr inline _t ceil(const _f& fvalue)
		{
			return static_cast<_t>(std::ceil(fvalue));
		}

		template <typename _t>
		inline _t cos(const _t& value)
		{
			return static_cast<_t>(std::cos(value));
		}

		inline float cosf(const float& value)
		{
			return cos(value);
		}

		template <typename _t>
		inline _t sin(const _t& value)
		{
			return static_cast<_t>(std::sin(value));
		}

		template <typename _t>
		inline _t asin(const _t& value)
		{
			return static_cast<_t>(std::asin(value));
		}

		inline float sinf(const float& value)
		{
			return sin(value);
		}

		inline float asinf(const float& value)
		{
			return sin(value);
		}

		template <typename _t>
		constexpr inline _t abs(const _t& value)
		{
			return static_cast<_t>(std::abs(value));
		}

		template <typename _t>
		constexpr inline _t lerp(const float t, const _t& min, const _t& max)
		{
			return min + ((max - min) * t);
		}

		template <typename _t>
		constexpr inline _t remap(const _t& value, const _t& oldMin, const _t& oldMax, const _t& newMin, const _t& newMax)
		{
			if (oldMax - oldMin == (_t)0) return (_t)0;
			return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
		}

		template <typename _t>
		constexpr inline _t pingpong(const _t& t, const _t& range)
		{
			_t range2 = range * 2;
			_t mod = std::fmod(t, range2);
			if (mod < range)
			{
				return mod;
			}
			else
			{
				return range2 - mod;
			}
		}

		// really just a sin...
		template <typename _t>
		constexpr inline _t pingpong(const _t& t, const _t& min, const _t& max)
		{
			const float intv = max - min;
			const float intv_half = (0.5f * intv);
			return min + intv_half + math::sin(t) * intv_half;
		}

		template <typename _t>
		constexpr inline _t is_zero(const _t& value)
		{
			return abs(value) < k_epsilon;
		}

#pragma region MinMax
#if 0
		template <typename _t>
		constexpr inline _t maximum(const _t& a, const _t& b, const bool chooseAWhenEqual = true)
		{
			if (a == b && chooseAWhenEqual) return a;
			else return b;

			if (a < b) return b;
			else return b;
		}

		template <typename _t>
		constexpr inline _t minimum(const _t& a, const _t& b, const bool chooseAWhenEqual = true)
		{
			if constexpr (a == b && chooseAWhenEqual) return a;
			else return b;

			if constexpr (a < b) return a;
			else return b;
		}
#endif
		template <typename _t>
		constexpr inline _t maximum(std::initializer_list<_t> list)
		{
			return std::max(list);
		}

		template <typename _t>
		constexpr inline _t minimum(std::initializer_list<_t> list)
		{
			return std::min(list);
		}

		template <typename _t>
		constexpr inline _t minimum(const _t& a, const _t& b)
		{
			return std::min(a, b);
		}

		template <typename _t>
		constexpr inline _t maximum(const _t& a, const _t& b)
		{
			return std::max(a, b);
		}

#if 0
		namespace detail
		{
			template <typename _t>
			constexpr _t const& do_maximum(_t const& v) { return v; }

			template <typename _t, class... _R>
			constexpr _t const& do_maximum(_t const& v0, _t const& v1, _R const&... rest)
			{
				return do_maximum(v0 < v1 ? v1 : v0, rest...);
			}

			template <typename _t>
			constexpr _t const& do_minimum(_t const& v) { return v; }

			template <typename _t, class... _R>
			constexpr _t const& do_minimum(_t const& v0, _t const& v1, _R const&... rest)
			{
				return do_minimum(v0 < v1 ? v0 : v1, rest...);
			}
		}
		
		template <typename _t, class... _R>
		inline constexpr _t const& maximum(_t const& a, _R const&... rest)
		{
			return detail::do_maximum(a, rest...);
		}

		template <typename _t, class... _R>
		inline constexpr _t const& minimum(_t const& a, _R const&... rest)
		{
			return detail::do_minimum(a, rest...);
		}
#endif
#pragma endregion

		template <typename _t>
		constexpr inline _t mid_of_three(const _t& a, const _t& b, const _t& c, const bool chooseMinWhenEqual = true)
		{
			constexpr _t min = minimum(a, b, c);
			constexpr _t max = maximum(a, b, c);

			if constexpr (a != min && a != max) return a;
			if constexpr (b != min && b != max) return b;
			if constexpr (c != min && c != max) return c;

			// There's an equality...
			return min;
		}

		template <typename _t>
		constexpr _t clamp(const _t& x, const _t& lo, const _t& hi) noexcept
		{
			return x < lo ? lo : (x > hi ? hi : x);
		}

		template <typename _t, typename = std::enable_if_t<std::is_unsigned_v<std::decay_t<_t>>>>
		constexpr _t bitwise_clamp(_t x, _t lo, _t hi) noexcept
		{
			return (x & hi) | lo;
		}

		template <typename _t>
		constexpr inline _t to_radians(_t degrees)
		{
			return degrees * (_t)(k_PI / 180.0);
		}

		template <typename _t>
		constexpr inline _t to_degrees(_t radians)
		{
			return radians * (_t)(180.0 / k_PI);
		}
	}
}
#endif


