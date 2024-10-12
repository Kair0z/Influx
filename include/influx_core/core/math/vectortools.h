#pragma once

#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/geometry/circle.h"

namespace influx::math
{
	inline influx::vector<math::vectorf3> get_points_in_circle(const math::circlef3D& circle, uint32 num_points)
	{
		influx::vector<math::vectorf3> result{};
		result.reserve(num_points);

		for (uint32 i = 0u; i < num_points; ++i)
		{
			const float angle_degrees = ((float)360 / num_points) * i;
			result.push_back(circle.get_point_at_degrees(angle_degrees));
		}

		return result;
	}

	inline influx::vector <math::vectorf3> get_normals_in_circle(const math::circlef3D& circle, uint32 num_points)
	{
		influx::vector<math::vectorf3> result{};
		result.reserve(num_points);

		for (uint32 i = 0u; i < num_points; ++i)
		{
			const float angle_degrees = ((float)360 / num_points) * i;
			result.push_back(circle.get_normal_at_degrees(angle_degrees));
		}

		return result;
	}
}