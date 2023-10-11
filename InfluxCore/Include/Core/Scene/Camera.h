#pragma once

#ifndef __CORE_SCENE_CAMERA_H_
#define __CORE_SCENE_CAMERA_H_

#include "core/math/transform.h"
#include "core/macros.h"

namespace influx::scene
{
	class camera final
	{
	public:
		camera() = default;
		camera(float fov, float nearp = 0.0f, float farp = 1.0f)
			: m_fov{ fov }, m_nearplane{ nearp }, m_farplane{ farp } {}

	private:
		FLX_CORE_GET_SET(float, fov);
		FLX_CORE_GET_SET(float, nearplane);
		FLX_CORE_GET_SET(float, farplane);
	};
}

#endif