#pragma once

#ifndef _CORE_GEOMETRY_TRIANGLE_H_
#define _CORE_GEOMETRY_TRIANGLE_H_

#include "core/math/vector.h"

namespace influx::math
{
	namespace detail
	{
		using dim_t = uint8;
	}

	template <typename _t, detail::dim_t _dim = 3u>
	struct triangle final
	{
	private:
		using point = vector<_t, _dim>;

	public:
		inline triangle() = default;
		inline triangle(const point& p0, const point& p1, const point& p2)
		{
			m_a = p0;
			m_b = p1;
			m_c = p2;
		}

		union
		{
			point m_points[3u];

			point m_a;
			point m_b;
			point m_c;
		};
	};

	template <typename _t>
	using triangle2D	= triangle<_t, 2u>;
	using triangle2Df	= triangle2D<float>;
	using triangle2Dd	= triangle2D<double>;
	using triangle2Di	= triangle2D<int>;
	using triangle2Du	= triangle2D<uint32>;

	template <typename _t>
	using triangle3D = triangle<_t, 3u>;
	using trianglef  = triangle3D<float>;
	using triangled  = triangle3D<double>;
	using trianglei  = triangle3D<int>;
	using triangleu  = triangle3D<uint32>;
}

#endif