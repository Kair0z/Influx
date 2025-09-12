#pragma once

#include "core/macros.h"
#include "core/math/matrix.h"

namespace influx
{
	class camera final
	{
	public:
		camera() = default;
		camera(float fov, float nearp = 0.0f, float farp = 1.0f)
			: m_fov{ fov }, m_nearplane{ nearp }, m_farplane{ farp } {}

		inline math::matrix4x4f get_projection(
			const float min_depth = 0.0f, 
			const float max_depth = 1.0f) const
		{
			return math::matrix4x4f::make_projection_RH(
				get_fov(),
				get_aspect_ratio(),
				get_nearplane(),
				get_farplane(),
				min_depth,
				max_depth);
		}

		/* 
			returns the inverse(transform) AND inverts the z-axis!
			this is because convention says camera space is -z aligned.
			iow, if an object in camera space is z=-1, it's in front of the camera.
		*/
		inline math::matrix4x4f get_view(const math::matrix4x4f& transform) const
		{
			math::matrix4x4f result = transform.inverted();
			result.set_column(2u, -result.get_column(2u));
			return result;
		}

	private:
		influx_property_readwrite(float, fov);
		influx_property_readwrite(float, nearplane);
		influx_property_readwrite(float, farplane);
		influx_property_readwrite(bool, is_orthographic);
		influx_property_readwrite(float, aspect_ratio);
	};
}