#pragma once

#include "core/math/vector.h"

namespace influx::math
{
	class ray final
	{
	public:
		ray() = default;
		ray(const math::vectorf3& origin, const math::vectorf3& direction, float min = 0.0f, float max = FLT_MAX)
			: m_origin{ origin }, m_direction{ direction }, m_min{ min }, m_max{ max }{}

		inline const math::vectorf3& get_origin() const
		{
			return m_origin;
		}

		inline const math::vectorf3& get_direction() const
		{
			return m_direction;
		}

		inline const float get_minimum() const
		{
			return m_min;
		}

		inline const float get_maximum() const
		{
			return m_max;
		}

	public:
		math::vectorf3 m_origin;
		math::vectorf3 m_direction;
		float m_min;
		float m_max;
	};
}