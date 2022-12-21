#pragma once

#ifndef _CORE_GEOMETRY_PLANE_H_
#define _CORE_GEOMETRY_PLANE_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	template <typename _T>
	struct Plane final
	{
	private:
		using Vector3 = Vector<_T, 3u>;

	public:
		inline Plane(const Vector3& normal, float offset) : m_normal{ normal }, m_offset{ offset }{}

		Vector3 m_normal;
		_T m_offset;
	};
}

#endif