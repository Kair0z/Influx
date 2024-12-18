#pragma once

// influx::core
#include "core/math/matrix.h"
#include "core/math/vector.h"

namespace influx::renderer
{
	inline math::matrix4x4f make_viewprojection(
		const math::matrix4x4f& camera_transform,
		const float aspect_ratio,
		const float field_of_view,
		const float near_plane,
		const float far_plane)
	{
		// invert camera :) (engine is right handed, but d3d12 is left handed)
		auto copy = camera_transform;
		math::matrix4x4f::invert(copy); // <-- this is probably wrong
		copy.set_collumn(2u, -copy.get_collumn(2u));

		const math::matrix4x4f mat_view = copy;
		const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(field_of_view, aspect_ratio, near_plane, far_plane);
		return mat_view * mat_proj;
	}
}