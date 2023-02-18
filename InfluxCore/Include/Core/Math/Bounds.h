#pragma once

#ifndef _CORE_MATH_BOUNDS_H_
#define _CORE_MATH_BOUNDS_H_

#include "Core/BasicTypes.h"
#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"
#include "Core/Container/Vector.h"

namespace Influx::Math
{
	namespace Internal
	{
		using dim_t = size_t;
	}

	template <typename _T, Internal::dim_t _N>
	class Bounds final
	{
	public:
		using Point = Math::Vector<_T, _N>;
		Bounds(const Influx::Vector<Point>& points);

	private:
		Point m_min;
		Point m_max;

	public:
		const Point GetMid() const;
		const Point& GetMin() const;
		const Point& GetMax() const;

		void Reset();
		void GrowTo(const Point& point);
		void ShrinkTo(const Point& point);

		bool Contains(const Point& point) const;
		bool Contains(const Point& point, Math::Vector<bool, _N>& out_isContained) const;

		enum class ContainState : uint8 { Less, Contained, More, Max };
		bool Contains(const Point& point, Math::Vector<ContainState, _N>& out_results) const;

		Bounds() = default;
		Bounds(const Bounds&) = default;
		Bounds(Bounds&&) = default;
		Bounds& operator=(const Bounds&) = default;
		Bounds& operator=(Bounds&&) = default;
		virtual ~Bounds() = default;
	};

	template <typename _T>
	using Rect  = Bounds<_T, 2u>;
	using Rectf = Rect<float>;
	using Recti = Rect<int>;
	using Rectu = Rect<uint32>;

	template <typename _T>
	using Box	= Bounds<_T, 3u>;
	using Boxf	= Box<float>;
	using Boxi  = Box<float>;
	using Boxu  = Box<uint32>;

	template<typename _T, Internal::dim_t _N>
	inline Bounds<_T, _N>::Bounds(const Influx::Vector<Bounds<_T, _N>::Point>& points)
	{
		for (Internal::dim_t i = 0; i < points.size(); ++i)
			GrowTo(points[i]);
	}

	template<typename _T, Internal::dim_t _N>
	inline const Bounds<_T, _N>::Point Bounds<_T, _N>::GetMid() const
	{
		return (m_max - m_min) / 2.0f;
	}

	template<typename _T, Internal::dim_t _N>
	inline const Bounds<_T, _N>::Point& Bounds<_T, _N>::GetMin() const
	{
		return m_min;
	}

	template<typename _T, Internal::dim_t _N>
	inline const Bounds<_T, _N>::Point& Bounds<_T, _N>::GetMax() const
	{
		return m_max;
	}

	template<typename _T, Internal::dim_t _N>
	inline void Bounds<_T, _N>::Reset()
	{
		m_min = m_max = Influx::Math::Vector<_T, _N>{};
	}

	template<typename _T, Internal::dim_t _N>
	inline void Bounds<_T, _N>::GrowTo(const Bounds<_T, _N>::Point& point)
	{
		Math::Vector<ContainState, _N> contained_results{};
		if (Contains(point, contained_results))
		{
			return;
		}

		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			if (contained_results[d] == ContainState::Less)
			{
				m_max[d] = point[d];
			}
		}
	}

	template<typename _T, Internal::dim_t _N>
	inline void Bounds<_T, _N>::ShrinkTo(const Bounds<_T, _N>::Point& point)
	{
		Math::Vector<ContainState, _N> contained_results{};
		if (!Contains(point, contained_results))
		{
			return;
		}

		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			if (contained_results[d] == ContainState::More)
			{
				m_max[d] = point[d];
			}
		}
	}

	template<typename _T, Internal::dim_t _N>
	inline bool Bounds<_T, _N>::Contains(const Point& point) const
	{
		Math::Vector<bool, _N>& out_isContained{};
		return Contains(point, out_isContained);
	}

	template<typename _T, Internal::dim_t _N>
	inline bool Bounds<_T, _N>::Contains(const Point& point, Math::Vector<bool, _N>& out_isContained) const
	{
		Math::Vector<ContainState, _N>& results{};
		const bool isContained = Contains(point, results);

		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			out_isContained[d] = (results[d] == ContainState::Contained);
		}

		return isContained;
	}

	template<typename _T, Internal::dim_t _N>
	inline bool Bounds<_T, _N>::Contains(const Point& point, Math::Vector<Bounds<_T, _N>::ContainState, _N>& out_results) const
	{
		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			out_results[d] = ContainState::Contained;
		}

		bool isBiggerThanMin = true;
		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			const bool isDimBiggerThanMin = (point[d] >= m_min[d]);
			
			if (!isDimBiggerThanMin)
			{
				out_results[d] = ContainState::Less;
			}

			isBiggerThanMin &= isDimBiggerThanMin;
		}

		bool isSmallerThanMax = true;
		for (Internal::dim_t d = 0; d < _N; ++d)
		{
			const bool isDimSmallerThanMax = (point[d] >= m_min[d]);
			
			if (!isDimSmallerThanMax)
			{
				out_results[d] = ContainState::More;
			}

			isSmallerThanMax &= isDimSmallerThanMax;
		}

		return (isBiggerThanMin && isSmallerThanMax);
	}
}

#endif