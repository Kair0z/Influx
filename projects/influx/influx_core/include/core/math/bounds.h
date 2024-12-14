#pragma once

#ifndef _CORE_MATH_BOUNDS_H_
#define _CORE_MATH_BOUNDS_H_

#include "core/basetypes.h"
#include "core/math/math.h"
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::math
{
	namespace detail
	{
		using dim_t = size_t;
	}

	template <typename _t, detail::dim_t _dim>
	class bounds final
	{
	public:
		using point = math::vector<_t, _dim>;
		bounds(const influx::vector<point>& points);

	private:
		point m_min;
		point m_max;

	public:
		const point get_middle() const;
		const point& get_minimum() const;
		const point& get_maximum() const;

		// sets to zero volume
		void reset();

		void grow_to_contain(const point& point);
		void shrink_to_contain(const point& point);

		bool contains(const point& point) const;
		bool contains(const point& point, math::vector<bool, _dim>& out_isContained) const;

		enum class e_contain_status : uint8 { less, contained, more, count };
		bool contains(const point& point, math::vector<uint8, _dim>& out_results) const;

		bounds() = default;
		bounds(const bounds&) = default;
		bounds(bounds&&) = default;
		bounds& operator=(const bounds&) = default;
		bounds& operator=(bounds&&) = default;
		virtual ~bounds() = default;
	};

	template <typename _t>
	using box	= bounds<_t, 3u>;
	using boxf	= box<float>;
	using boxi  = box<float>;
	using boxu  = box<uint32>;

	template<typename _t, detail::dim_t _dim>
	inline bounds<_t, _dim>::bounds(const influx::vector<bounds<_t, _dim>::point>& points)
	{
		for (detail::dim_t i = 0; i < points.dimension(); ++i)
			grow_to_contain(points[i]);
	}

	template<typename _t, detail::dim_t _dim>
	inline const bounds<_t, _dim>::point bounds<_t, _dim>::get_middle() const
	{
		return (m_max - m_min) / 2.0f;
	}

	template<typename _t, detail::dim_t _dim>
	inline const bounds<_t, _dim>::point& bounds<_t, _dim>::get_minimum() const
	{
		return m_min;
	}

	template<typename _t, detail::dim_t _dim>
	inline const bounds<_t, _dim>::point& bounds<_t, _dim>::get_maximum() const
	{
		return m_max;
	}

	template<typename _t, detail::dim_t _dim>
	inline void bounds<_t, _dim>::reset()
	{
		m_min = m_max = influx::math::vector<_t, _dim>{};
	}

	template<typename _t, detail::dim_t _dim>
	inline void bounds<_t, _dim>::grow_to_contain(const bounds<_t, _dim>::point& point)
	{
		math::vector<uint8, _dim> results{};
		if (contains(point, results))
		{
			return;
		}

		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			if (results[d] == (uint8)e_contain_status::less)
			{
				m_max[d] = point[d];
			}
		}
	}

	template<typename _t, detail::dim_t _dim>
	inline void bounds<_t, _dim>::shrink_to_contain(const bounds<_t, _dim>::point& point)
	{
		math::vector<uint8, _dim> results{};
		if (!contains(point, results))
		{
			return;
		}

		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			if (results[d] == (uint8)e_contain_status::more)
			{
				m_max[d] = point[d];
			}
		}
	}

	template<typename _t, detail::dim_t _dim>
	inline bool bounds<_t, _dim>::contains(const point& point) const
	{
		math::vector<bool, _dim>& out_isContained{};
		return contains(point, out_isContained);
	}

	template<typename _t, detail::dim_t _dim>
	inline bool bounds<_t, _dim>::contains(const point& point, math::vector<bool, _dim>& out_isContained) const
	{
		math::vector<e_contain_status, _dim>& results{};
		const bool isContained = contains(point, results);

		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			out_isContained[d] = (results[d] == e_contain_status::contained);
		}

		return isContained;
	}

	template<typename _t, detail::dim_t _dim>
	inline bool bounds<_t, _dim>::contains(const point& point, math::vector<uint8, _dim>& out_results) const
	{
		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			out_results[d] = (uint8)e_contain_status::contained;
		}

		bool isBiggerThanMin = true;
		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			const bool isDimBiggerThanMin = (point[d] >= m_min[d]);
			
			if (!isDimBiggerThanMin)
			{
				out_results[d] = (uint8)e_contain_status::less;
			}

			isBiggerThanMin &= isDimBiggerThanMin;
		}

		bool isSmallerThanMax = true;
		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			const bool isDimSmallerThanMax = (point[d] >= m_min[d]);
			
			if (!isDimSmallerThanMax)
			{
				out_results[d] = (uint8)e_contain_status::more;
			}

			isSmallerThanMax &= isDimSmallerThanMax;
		}

		return (isBiggerThanMin && isSmallerThanMax);
	}
}

#endif