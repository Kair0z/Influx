#pragma once

#ifndef _CORE_SCENE_CAMERA_H_
#define _CORE_SCENE_CAMERA_H_

#include "Core/Math/Vector.h"

namespace Influx::Scene
{
	class Camera final
	{
	public:
		inline Camera() = default;
		inline Camera(const Vectorf3& position, const Vectorf3& forward, const Vectorf3& up = Vectorf3{ 0.0f, 1.0f, 0.0f })
			: m_position{position}
			, m_forward{forward}
			, m_up{up}
		{
			RecalculateTransform();
		}

		inline void SetFieldOfView(float newFov)
		{
			m_fieldOfView = newFov;
		}

		inline float GetFieldOfView() const
		{
			return m_fieldOfView;
		}

		inline void SetPosition(const Vectorf3& newPosition)
		{
			m_position = newPosition;
		}

		inline void SetForward(const Vectorf3& newForward)
		{
			m_forward = newForward.Normalized();
			RecalculateTransform();
		}

		inline Vectorf3 GetPosition() const
		{
			return m_position;
		}

		inline Vectorf3 GetForward() const
		{
			return m_forward;
		}

		inline Vectorf3 GetRight() const
		{
			return m_right;
		}

		inline Vectorf3 GetUp() const
		{
			return m_up;
		}

	private:
		Vectorf3 m_forward;
		Vectorf3 m_right;
		Vectorf3 m_up;
		Vectorf3 m_position;

		float m_fieldOfView;

	private:
		inline void RecalculateTransform()
		{
			m_right = Vectorf3::Cross(m_forward, m_up);

			//m_worldViewProjection =
			//	Matrix4x4f::MakeTransformMatrixRH(m_position, m_forward, m_up) * 
			//	Matrix4x4f::MakeViewMatrixRH(m_position, m_forward, m_up) * 
			//	Matrix4x4f::MakeProjectionMatrixRH(90.0f, 1.0f, 0.0f, 100.0f);
		}
	};
}

#endif