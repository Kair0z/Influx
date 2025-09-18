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
		const math::matrix4x4f mat_view = math::matrix4x4f::make_view(camera_transform);
		const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(field_of_view, aspect_ratio, near_plane, far_plane);
		return mat_view * mat_proj;
	}
}