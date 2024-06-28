#pragma once

#ifndef _CORE_GEOMETRY_PLANE_H_
#define _CORE_GEOMETRY_PLANE_H_

#include "core/math/vector.h"

namespace influx::math
{
	template <typename _t>
	struct plane final
	{
	private:
		using vector3 = math::vector<_t, 3u>;

	public:
		inline plane(const vector3& normal, float offset) : m_normal{ normal }, m_offset{ offset }{}

		vector3 get_origin() const;

		vector3 m_normal;
		_t m_offset;
	};

	using planef = plane<float>;

	template<typename _t>
	inline plane<_t>::vector3 plane<_t>::get_origin() const
	{
		return m_normal * m_offset;
	}
}

#endif