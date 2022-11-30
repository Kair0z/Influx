#pragma once

#ifndef _CORE_GEOMETRY_RECT_H_
#define _CORE_GEOMETRY_RECT_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	template <typename _T>
	struct Rect final
	{
	private:
		using Vector2 = Vector<_T, 2u>;

	public:
		inline Rect(_T l, _T b, _T w, _T h) : m_leftBottom{ l,b }, m_widthHeigth{ w,h }{}
		inline Rect(const Vector2& lb, const Vector2& wh) : m_leftBottom{ lb }, m_widthHeigth{ wh }{}

		Vector2 m_leftBottom{};
		Vector2 m_widthHeigth{};
	};
}

#endif