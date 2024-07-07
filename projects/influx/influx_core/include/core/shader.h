#pragma once

#include "basetypes.h"

namespace influx
{
	enum class e_shader_type : uint8
	{
		vs,
		ps,
		count
	};

	enum class e_shader_target : uint8
	{
		_6_2,
		count
	};
}