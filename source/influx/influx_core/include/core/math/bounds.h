#pragma once

#ifndef _CORE_MATH_BOUNDS_H_
#define _CORE_MATH_BOUNDS_H_

#include "core/basetypes.h"
#include "core/math/math.h"
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/math/matrix.h"
#include "core/math/ray.h"

// STL
#include <algorithm> // swap

namespace influx::math
{
	namespace detail
	{
		using dim_t = uint32;
	}

	template <typename _t, detail::dim_t _dim>
	class bounds final
	{
	public:
		using point = math::vector<_t, _dim>;
		bounds(const influx::vector<point>& points);
		bounds(const point& min, const point& max)
			: m_min{ min }, m_max{ max }{}

	private:
		point m_min;
		point m_max;

	public:
		const point get_middle() const;
		const point& get_minimum() const;
		const point& get_maximum() const;

		static const bounds& identity()
		{
			static bounds g_identity{ -point::make_one(), point::make_one() };
			return g_identity;
		}

		// sets to zero volume
		void reset();

		void grow_to_contain(const point& point);
		void shrink_to_contain(const point& point);

		bool contains(const point& point) const;
		bool contains(const point& point, math::vector<bool, _dim>& out_isContained) const;

		enum class e_contain_status : uint8 { less, contained, more, count };
		bool contains(const point& point, math::vector<uint8, _dim>& out_results) const;

		bounds get_scaled(const float scale) const
		{
			return bounds{ m_min * scale, m_max * scale };
		}
		bounds get_scaled(const math::vector<_t, _dim>& scale) const
		{
			return bounds{ m_min * scale, m_max * scale };
		}
		bounds get_transformed3D(const math::matrix<_t, 4u, 4u>& matrix) const
		{
			return bounds
			{
				matrix * m_max,
				matrix * m_min
			};
		}

		inline bool trace(const ray& ray, float& out_distance) const
		{
			const auto& ray_origin = ray.get_origin();
			const auto& ray_direction = ray.get_direction();
			float t_min = (m_min[0] - ray_origin[0]) / ray_direction[0]; 
			float t_max = (m_max[0] - ray_origin[0]) / ray_direction[0]; 

			if (t_min > t_max)
			{
				std::swap(t_min, t_max);
			}
			
			for (uint32 i = 1u; i < 3u; ++i) 
			{  
				// Process y and z axes
				float t1 = (m_min[i] - ray_origin[i]) / ray_direction[i];
				float t2 = (m_max[i] - ray_origin[i]) / ray_direction[i];

				if (t1 > t2)
				{
					std::swap(t1, t2);
				}

				t_min = math::maximum(t_min, t1);
				t_max = math::minimum(t_max, t2);

				if (t_min > t_max)
				{
					return false;
				}
			}

			out_distance = t_min;
			return true;
		}

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
			switch (results[d])
			{
			case (uint8)e_contain_status::less: m_min[d] = point[d]; break;
			case (uint8)e_contain_status::more: m_max[d] = point[d]; break;
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

		bool min_exceeded = false;
		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			const bool is_smaller_than_min = (point[d] < m_min[d]);
			if (is_smaller_than_min)
			{
				out_results[d] = (uint8)e_contain_status::less;
			}

			min_exceeded |= is_smaller_than_min;
		}

		bool max_exceeded = false;
		for (detail::dim_t d = 0; d < _dim; ++d)
		{
			const bool is_bigger_than_max = point[d] > m_max[d];
			if (is_bigger_than_max)
			{
				out_results[d] = (uint8)e_contain_status::more;
			}

			max_exceeded |= is_bigger_than_max;
		}

		return (!min_exceeded && !max_exceeded);
	}
}

#endif