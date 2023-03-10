#pragma once

#ifndef __CORE_GEOMETRY_VERTEX_H_
#define __CORE_GEOMETRY_VERTEX_H_

#include "Core/Math/Vector.h"

namespace Influx::Math
{
	struct Vertex final
	{
		Vertex() = default;
		Vertex(const Vectorf3& position, const Vectorf4& colour, const Vectorf3& normal, const Vectorf2& uv)
			: Position{ position }, Colour{ colour }, Normal{ normal }, UV{ uv } {};

		Vectorf3 Position{};
		Vectorf4 Colour{};
		Vectorf3 Normal{};
		Vectorf2 UV{};
	};
}

#endif


