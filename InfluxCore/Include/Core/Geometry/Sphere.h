#pragma once

#ifndef _CORE_GEOMETRY_SPHERE_H_
#define _CORE_GEOMETRY_SPHERE_H_

#include "core/math/vector.h"

namespace influx::math
{
	template <typename _t>
	struct sphere final
	{
	private:
		using vector3 = math::vector<_t, 3u>;

	public:
		inline sphere() = default;
		inline sphere(const vector3& position, const _t radius)
			: m_radius{ radius }, m_position{ position }{}

		_t		m_radius;
		vector3 m_position;
	};

	using spheref = sphere<float>;
}

#endif