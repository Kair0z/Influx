#pragma once

#include "core/basetypes.h"
#include "Core/Math/Vector.h"

namespace influx::math
{
	using colour_rgba = vectorf4;
}

namespace influx::colour
{
	const static math::colour_rgba k_white{ 1.0f, 1.0f, 1.0f, 1.0f };
}