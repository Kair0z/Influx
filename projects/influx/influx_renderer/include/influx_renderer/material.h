#pragma once 

#include "core/math/vector.h"
#include "core/string.h"

namespace influx::renderer
{
	struct material final
	{
		math::vectorf4 m_albedo{};
	};
}