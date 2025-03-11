#pragma once

#include "core/basetypes.h"
#include "Core/Math/Vector.h"
#include "core/math/random.h"

namespace influx::math
{
	using colour_rgba = vectorf4;
}

namespace influx::colour
{
	const static math::colour_rgba k_white{ 1.0f, 1.0f, 1.0f, 1.0f };
	const static math::colour_rgba k_red{ 1.0f, 0.0f, 0.0f, 1.0f };
	const static math::colour_rgba k_blue{ 0.0f, 0.0f, 1.0f, 1.0f };
	const static math::colour_rgba k_green{ 0.0f, 1.0f, 0.0f, 1.0f };
	const static math::colour_rgba k_black{ 0.0f, 0.0f, 0.0f, 1.0f };
	const static math::colour_rgba k_gray{ 0.2f, 0.2f, 0.2f, 1.0f };

	static math::colour_rgba random()
	{
		auto random_vec = random::get_random_unit_vectorf3();
		return math::colour_rgba(random_vec.x, random_vec.y, random_vec.z, 1.0f);
	}
}