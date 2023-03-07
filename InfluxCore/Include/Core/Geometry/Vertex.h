#pragma once

#ifndef __CORE_GEOMETRY_VERTEX_H_
#define __CORE_GEOMETRY_VERTEX_H_

#include "Core/Math/Vector.h"

namespace Influx::Math
{
	struct Vertex final
	{
		Vectorf3 Position{};
		Vectorf4 Color{};
		Vectorf3 Normal{};
		Vectorf2 UV{};
	};
}

#endif


