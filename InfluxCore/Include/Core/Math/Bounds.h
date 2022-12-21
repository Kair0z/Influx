#pragma once

#ifndef _CORE_MATH_BOUNDS_H_
#define _CORE_MATH_BOUNDS_H_

#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"
#include "Core/Container/Vector.h"

namespace Influx::Math
{
	template <typename _T, size_t _N>
	class Bounds final
	{
	public:
		using VectorType = Math::Vector<_T, _N>;
		Bounds(const Influx::Vector<VectorType>& points);

	private:
		VectorType m_min;
		VectorType m_max;

	public:
		const VectorType GetMid() const;
		const VectorType& GetMin() const;
		const VectorType& GetMax() const;

		void Reset();
		void AddPoint(const VectorType& point);

		Bounds() = default;
		Bounds(const Bounds&) = default;
		Bounds(Bounds&&) = default;
		Bounds& operator=(const Bounds&) = default;
		Bounds& operator=(Bounds&&) = default;
		virtual ~Bounds() = default;
	};

	template<typename _T, size_t _N>
	inline Bounds<_T, _N>::Bounds(const Influx::Vector<Bounds<_T, _N>::VectorType>& points)
	{
		for (size_t i = 0; i < points.size(); ++i)
			AddPoint(points[i]);
	}

	template<typename _T, size_t _N>
	inline const Bounds<_T, _N>::VectorType Bounds<_T, _N>::GetMid() const
	{
		return (m_max - m_min) / 2.0f;
	}

	template<typename _T, size_t _N>
	inline const Bounds<_T, _N>::VectorType& Bounds<_T, _N>::GetMin() const
	{
		return m_min;
	}

	template<typename _T, size_t _N>
	inline const Bounds<_T, _N>::VectorType& Bounds<_T, _N>::GetMax() const
	{
		return m_max;
	}

	template<typename _T, size_t _N>
	inline void Bounds<_T, _N>::Reset()
	{
		m_min = m_max = Influx::Math::Vector<_T, _N>{};
	}

	template<typename _T, size_t _N>
	inline void Bounds<_T, _N>::AddPoint(const Bounds<_T, _N>::VectorType& point)
	{
		for (size_t d = 0; d < _N; ++d)
		{
			m_min[d] = Influx::Math::Min(m_min[d], point[d]);
			m_max[d] = Influx::Math::Max(m_max[d], point[d]);
		}
	}
}

#endif