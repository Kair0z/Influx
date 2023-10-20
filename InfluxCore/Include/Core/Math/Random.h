#pragma once

#ifndef __CORE_RANDOM_H_
#define __CORE_RANDOM_H_

#include "core/container/vector.h"
#include "core/math/vector.h"
#include "core/geometry/sphere.h"
#include "Core/Math/Math.h"
#include <ctime>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace influx::random
{
	template <typename _t>
	constexpr inline _t get_tent_random()
	{
		float r = 2 * (float)rand() / RAND_MAX;
		return (r < 1) ? sqrt(r) - 1 : 1 - sqrt(2 - r);
	}

	inline void seed_random()
	{
		std::srand(unsigned int(std::time(nullptr)));
	}

	inline void seed_random(unsigned int seed)
	{
		std::srand(seed);
	}

	template <typename _t>
	inline _t get_random(const _t& min = std::numeric_limits<_t>::min(), const _t& max = std::numeric_limits<_t>::max())
	{
		return static_cast<_t>(min + (std::rand() / (RAND_MAX / (max - min))));
	}

	template <typename _t, size_t _N>
	inline vector<_t> get_randoms(const _t& min = std::numeric_limits<_t>::min(), const _t& max = std::numeric_limits<_t>::max())
	{
		vector<_t> randoms(_N);
		for (size_t i = 0; i < _N; ++i)
			randoms[i] = get_random<_t>(min, max);

		return randoms;
	}

#pragma region math vector
	inline math::vector<float, 3u> get_random_unit_vectorf3()
	{
		math::vector<float, 3u> result{1.0f, 0.0f};
		float phi = get_random<float>(0.0f, math::k_PIDouble);
		float costheta = get_random<float>(-1.0f, 1.0f);
		float theta = std::acosf(costheta);

		result.x = std::sinf( theta) * std::cosf( phi );
		result.y = std::sinf( theta) * std::sinf( phi );
		result.z = std::cosf( theta );
		
		return result;
	}

	template <size_t _n>
	inline vector<math::vector<float, 3u>> get_random_unit_vectorf3s()
	{
		vector<math::vector<float, 3u>> result{};
		for (size_t i = 0u; i < _n; ++i)
		{
			result.push_back(get_random_unit_vectorf3());
		}
		return result;
	}
#pragma endregion
	
#pragma region spheres
	inline math::spheref get_random_spheref(
		const math::vectorf2& minMaxDistance,
		const math::vectorf2& minMaxRadius)
	{
		math::spheref sphere{};
		sphere.m_position = get_random_unit_vectorf3() * get_random<float>(minMaxDistance.x, minMaxDistance.y);
		sphere.m_radius = get_random(minMaxRadius.x, minMaxRadius.y);
		return sphere;
	}

	template <size_t _N>
	inline vector<math::spheref> get_random_spherefs(
		const math::vectorf2& minMaxDistance,
		const math::vectorf2& minMaxRadius)
	{
		influx::vector<math::spheref> randoms(_N);
		for (size_t i = 0; i < _N; ++i)
		{
			randoms.push_back(get_random_spheref(minMaxDistance, minMaxRadius));
		}

		return randoms;
	}
#pragma endregion
}

#endif