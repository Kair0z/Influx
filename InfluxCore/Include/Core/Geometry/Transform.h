#pragma once

#ifndef _CORE_GEOMETRY_TRANSFORM_H_
#define _CORE_GEOMETRY_TRANSFORM_H_

#include "../Math/Vector.h"

namespace Influx::Math
{
	struct Transform2D final
	{
		inline Transform2D(const Vectorf2& position, float rotation, const Vectorf2& scale)
			: Position{ position }, Rotation{ rotation }, Scale{ scale }{}

		Vectorf2 Position;
		Vectorf2 Scale;
		float Rotation;
	};

	struct Transform3D final
	{
		inline Transform3D(const Vectorf3& position, const Vectorf3& rotation, const Vectorf3& scale)
			: Position{ position }, Rotation{ rotation }, Scale{ scale }{}

		Vectorf3 Position;
		Vectorf3 Rotation;
		Vectorf3 Scale;
	};
}

#endif