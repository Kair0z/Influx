#pragma once 
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::renderer
{
	struct material_data final
	{
		math::vectorf4 m_albedo{};
	};
}