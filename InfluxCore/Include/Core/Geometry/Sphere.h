#pragma once

#ifndef _CORE_GEOMETRY_SPHERE_H_
#define _CORE_GEOMETRY_SPHERE_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	template <typename _T>
	struct Sphere final
	{
	private:
		using Vector3 = Vector<_T, 3u>;

	public:
		inline Sphere() = default;
		inline Sphere(const Vector3& position, const _T radius)
			: m_radius{ radius }, m_position{ position }{}

		_T		m_radius;
		Vector3 m_position;
	};

	using Spheref = Sphere<float>;
}

#endif