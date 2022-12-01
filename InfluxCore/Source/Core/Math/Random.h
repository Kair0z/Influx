#pragma once

#ifndef _CORE_RANDOM_H_
#define _CORE_RANDOM_H_

#include "Core/Container/Vector.h"
#include "Core/Math/Vector.h"
#include "Core/Geometry/Sphere.h"

#include <time.h>

namespace Influx::Random
{
	template <typename _T>
	constexpr inline _T TentRandom()
	{
		float r = 2 * (float)rand() / RAND_MAX;
		return (r < 1) ? sqrt(r) - 1 : 1 - sqrt(2 - r);
	}

	inline void SeedRandom()
	{
		std::srand(unsigned int(time(nullptr)));
	}

	inline void SeedRandom(unsigned int seed)
	{
		std::srand(seed);
	}

	template <typename _T>
	inline _T Random(const _T& min = std::numeric_limits<_T>::min(), const _T& max = std::numeric_limits<_T>::max())
	{
		return static_cast<_T>(min + (std::rand() / (RAND_MAX / (max - min))));
	}

	template <typename _T, size_t _N>
	inline std::vector<_T> Randoms(const _T& min = std::numeric_limits<_T>::min(), const _T& max = std::numeric_limits<_T>::max())
	{
		Vector<_T> randoms(_N);
		for (size_t i = 0; i < _N; ++i)
			randoms[i] = Random<_T>(min, max);

		return randoms;
	}

	namespace Vector
	{
		template <typename _T, Math::VectorSizeType _N>
		inline Math::Vector<_T, _N> Random()
		{
			Math::Vector<_T, _N> result{};
			for (Math::VectorSizeType i = 0; i < _N; ++i)
				result[i] = Random<_T>();

			return result;
		}

		template <typename _T, Math::VectorSizeType _N>
		inline Math::Vector<_T, _N> Random(const Math::Vector<_T, _N>& min, const Math::Vector<_T, _N>& max)
		{
			Math::Vector<_T, _N> result{};
			for (Math::VectorSizeType i = 0; i < _N; ++i)
				result[i] = Influx::Random::Random<_T>(min[i], max[i]);

			return result;
		}

		template <size_t _N, typename _T, Math::VectorSizeType _NN>
		inline std::vector<Math::Vector<_T, _NN>> Randoms(const Math::Vector<_T, _NN>& min, const Math::Vector<_T, _NN>& max)
		{
			using vector_type = Math::Vector<_T, _NN>;

			Influx::Vector<vector_type> randoms(_N);
			for (size_t i = 0; i < _N; ++i)
				randoms[i] = Random<_T, _NN>(min, max);

			return randoms;
		}

		template <size_t _N, typename _T, Math::VectorSizeType _NN>
		inline std::vector<Math::Vector<_T, _NN>> Randoms()
		{
			using vector_type = Math::Vector<_T, _NN>;

			constexpr _T numeric_max = std::numeric_limits<_T>::max();
			constexpr _T numeric_min = std::numeric_limits<_T>::min();

			return Randoms<_N, _T, _NN>(
				vector_type{ numeric_min, numeric_min, numeric_min }, 
				vector_type{ numeric_max, numeric_max, numeric_max });
		}

		template <size_t _N>
		inline std::vector<Math::Vector<float, 3u>> Random3fs(const Math::Vector<float, 3u>& min, const Math::Vector<float, 3u>& max)
		{
			return Randoms<_N, float, 3u>(min, max);
		}

		template <size_t _N>
		inline std::vector<Math::Vector<float, 2u>> Random2fs(const Math::Vector<float, 2u>& min, const Math::Vector<float, 2u>& max)
		{
			return Randoms<_N, float, 2u>(min, max);
		}
	}
	
	namespace Sphere
	{
		template <size_t _N>
		inline std::vector<Math::Sphere<float>> RandomSpherefs(
			const Math::Vectorf3& minPosition, 
			const Math::Vectorf3& maxPosition, 
			const Math::Vectorf2& minMaxRadius)
		{
			using sphere_type = Math::Sphere<float>;

			Influx::Vector<sphere_type> randoms(_N);
			for (size_t i = 0; i < _N; ++i)
			{
				randoms[i].m_position = Random::Vector::Random<float, 3u>(minPosition, maxPosition);
				randoms[i].m_radius = Random::Random(minMaxRadius.x, minMaxRadius.y);
			}

			return randoms;
		}
	}
}

#endif