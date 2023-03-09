#pragma once

#ifndef __CORE_SCENE_CAMERA_H_
#define __CORE_SCENE_CAMERA_H_

#include "Core/Math/Transform.h"

namespace Influx::Scene
{
	class Camera final
	{
	public:
		Camera() = default;

		void SetFieldOfView(float newFov)
		{
			m_fieldOfView = newFov;
		}

		float GetFieldOfView() const
		{
			return m_fieldOfView;
		}

	private:
		float m_fieldOfView;
	};
}

#endif