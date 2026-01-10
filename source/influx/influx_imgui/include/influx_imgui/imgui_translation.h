#pragma once

// influx::core
#include "core/math/vector.h"

// imgui
#include "imgui.h"

namespace influx::imgui
{
	inline constexpr static ImVec2 translate(const math::vectorf2& vec)
	{
		return ImVec2{ vec.x, vec.y };
	}

	inline static math::vectorf2 translate(const ImVec2& vec)
	{
		return math::vectorf2{ vec.x, vec.y };
	}
}