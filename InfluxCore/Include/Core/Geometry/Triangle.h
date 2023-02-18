#pragma once

#ifndef _CORE_GEOMETRY_TRIANGLE_H_
#define _CORE_GEOMETRY_TRIANGLE_H_

#include "Core/Math/Vector.h"

namespace Influx::Math
{
	namespace Internal
	{
		using dim_t = uint8;
	}

	template <typename _T, Internal::dim_t _N = 3u>
	struct Triangle final
	{
	private:
		using Point = Vector<_T, _N>;

	public:
		inline Triangle() = default;
		inline Triangle(const Point& p0, const Point& p1, const Point& p2)
		{
			A = p0;
			B = p1;
			C = p2;
		}

		union
		{
			Point Points[3u];

			Point A;
			Point B;
			Point C;
		};
	};

	template <typename _T>
	using Triangle2D	= Triangle<_T, 2u>;
	using Triangle2Df	= Triangle2D<float>;
	using Triangle2Dd	= Triangle2D<double>;
	using Triangle2Di	= Triangle2D<int>;
	using Triangle2Du	= Triangle2D<uint32>;

	template <typename _T>
	using Triangle3D = Triangle<_T, 3u>;
	using Trianglef  = Triangle3D<float>;
	using Triangled  = Triangle3D<double>;
	using Trianglei  = Triangle3D<int>;
	using Triangleu  = Triangle3D<uint32>;
}

#endif