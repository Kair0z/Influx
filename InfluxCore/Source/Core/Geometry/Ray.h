#pragma once

#ifndef _CORE_GEOMETRY_RAY_H_
#define _CORE_GEOMETRY_RAY_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	class Ray final
	{
	public:
		Ray(const Vectorf3& origin, const Vectorf3& direction, float min = 0.0f, float max = FLT_MAX) 
			: m_origin{ origin }, m_direction{ direction }, m_min{ min }, m_max{ max }{}

		inline const Vectorf3& GetOrigin() const
		{
			return m_origin;
		}

		inline const Vectorf3& GetDirection() const
		{
			return m_direction;
		}

		inline const float GetMin() const
		{
			return m_min;
		}

		inline const float GetMax() const
		{
			return m_max;
		}

	private:
		Vectorf3 m_origin;
		Vectorf3 m_direction;
		float m_min;
		float m_max;
	};
}

#endif