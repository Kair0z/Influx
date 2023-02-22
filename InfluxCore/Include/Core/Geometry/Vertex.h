#pragma once

#ifndef _CORE_GEOMETRY_VERTEX_H_
#define _CORE_GEOMETRY_VERTEX_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	struct Vertex
	{
		Vectorf3 Position{};
		Vectorf4 Color{};
	};
}

#endif


