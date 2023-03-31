#pragma once

#ifndef __CORE_SCENE_CAMERA_H_
#define __CORE_SCENE_CAMERA_H_

#include "Core/Math/Transform.h"
#include "Core/Macros.h"

namespace Influx::Scene
{
	class Camera final
	{
	public:
		Camera() = default;
		Camera(float fov, float near, float far)
			: Fov{ fov }, NearPlane{ near }, FarPlane{ far } {}

		FLX_CORE_GET_SET(float, Fov);
		FLX_CORE_GET_SET(float, NearPlane);
		FLX_CORE_GET_SET(float, FarPlane);
	};
}

#endif