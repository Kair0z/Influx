#pragma once

#ifndef _CORE_GEOMETRY_RECT_H_
#define _CORE_GEOMETRY_RECT_H_

#include "core/math/vector.h"

namespace influx::math
{
	template <typename _t>
	struct rect final
	{
	private:
		using vector2 = math::vector<_t, 2u>;

	public:
		inline rect(_t l, _t b, _t w, _t h) : m_leftBottom{ l,b }, m_width_height{ w,h }{}
		inline rect(const vector2& lb, const vector2& wh) : m_leftBottom{ lb }, m_width_height{ wh }{}

		const vector2& get_dimensions() const
		{
			return m_width_height;
		}

		vector2 m_leftBottom{};
		vector2 m_width_height{};
	};

	using rectf = rect<float>;
	using rectu = rect<uint32>;
	using recti = rect<int>;
	using rectd = rect<double>;
}

#endif