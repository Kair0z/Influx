#pragma once

#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/math/quad.h"
#include "core/function.h"

namespace influx::math::geometry
{
	inline static void traverse(const quadf& rectangle, 
		const function<void(const vectorf3&)>& func_vertex,
		const function<void(const uint32)>& func_index)
	{
		func_vertex(rectangle.get_point(rectf::e_point::left_bottom));
		func_vertex(rectangle.get_point(rectf::e_point::left_top));
		func_vertex(rectangle.get_point(rectf::e_point::right_top));
		func_vertex(rectangle.get_point(rectf::e_point::right_bottom));

		func_index(0);
		func_index(1);
		func_index(2);
		func_index(3);
	}
}