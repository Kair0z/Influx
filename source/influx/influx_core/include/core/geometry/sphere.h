#pragma once

#ifndef _CORE_GEOMETRY_SPHERE_H_
#define _CORE_GEOMETRY_SPHERE_H_

#include "Core/Math/Math.h"
#include "core/math/vector.h"

namespace influx::math
{
	template <typename _t>
	struct sphere final
	{
	private:
		using vector3 = math::vector<_t, 3u>;

	public:
		inline sphere()
			: m_radius{}, m_position{} {}

		inline sphere(const vector3& position, const _t radius)
			: m_radius{ radius }, m_position{ position }{}

		void grow_to(const vector3& point);
		void shrink_to(const vector3& point);

		_t		m_radius;
		vector3 m_position;
	};

	using spheref = sphere<float>;

	template <typename _t>
	void sphere<_t>::grow_to(const vector3& point)
	{
		vector3 diff = (point - m_position);
		if (diff.is_zero()) return;

		float distance = diff.magnitude();
		if (m_radius < distance)
		{
			m_radius = distance;
		}
	}

	template <typename _t>
	void sphere<_t>::shrink_to(const vector3& point)
	{
		vector3 diff = (point - m_position);
		if (diff.is_zero())
		{
			return;
		}

		float distance = diff.magnitude();
		if (m_radius > distance)
		{
			m_radius = distance;
		}
	}
}

#endif