#pragma once

#include "core/macros.h"
#include "core/math/matrix.h"

namespace influx::scene
{
	class camera final
	{
	public:
		camera() = default;
		camera(float fov, float nearp = 0.0f, float farp = 1.0f)
			: m_fov{ fov }, m_nearplane{ nearp }, m_farplane{ farp } {}

		inline math::matrix4x4f get_projection() const
		{
			return math::matrix4x4f::make_projection_RH(
				get_fov(),
				get_aspect_ratio(),
				get_nearplane(),
				get_farplane());
		}

	private:
		influx_property_readwrite(float, fov);
		influx_property_readwrite(float, nearplane);
		influx_property_readwrite(float, farplane);
		influx_property_readwrite(bool, is_orthographic);
		influx_property_readwrite(float, aspect_ratio);
	};
}