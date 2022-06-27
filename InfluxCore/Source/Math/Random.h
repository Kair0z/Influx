#pragma once

#ifndef _MATH_RANDOM_H_
#define _MATH_RANDOM_H_

#ifdef max
#undef max
#endif

#include "Math/Math.h"
#include <GLM/glm/gtc/random.hpp>

namespace Influx
{
	using namespace Math;
	namespace Random
	{
		inline Vector3f GetRandomPointInSphere(const float radius = 1.0f)
		{
			return glm::ballRand(radius);
		}
	}
}

#endif