#pragma once

#ifndef __CORE_MATH_TRANSFORM_H_
#define __CORE_MATH_TRANSFORM_H_

#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Quaternion.h"

namespace Influx::Math
{
	class Transformf2D final
	{
	public:
		Transformf2D() = default;
		Transformf2D(const Math::Vectorf2& position, float rotation, const Math::Vectorf2& scale)
			: m_position{ position }, m_rotation{ rotation }, m_scale{ scale } {}

		Transformf2D(const Transformf2D&) = default;
		Transformf2D(Transformf2D&&) = default;
		Transformf2D& operator=(const Transformf2D&) = default;
		Transformf2D& operator=(Transformf2D&&) = default;
		virtual ~Transformf2D() = default;

	private:
		Math::Vectorf2 m_position;
		Math::Vectorf2 m_scale;
		float m_rotation;
	};

	class Transformf3D final
	{
	public:
		Transformf3D() = default;
		Transformf3D(const Math::Vectorf3& position, const Math::Rotation& rotation, const Math::Vectorf3& scale)
			: m_position{ position }, m_rotation{ rotation }, m_scale { scale } {}
		
		// Position
		void SetPosition(const Vectorf3& position)
		{
			m_position = position;
		}

		Vectorf3 GetPosition() const
		{
			return m_position;
		}
		
		// Rotation
		Math::Vectorf3 GetForward() const
		{
			return m_rotation.GetForward();
		}

		Math::Vectorf3 GetRight() const
		{
			return m_rotation.GetRight();
		}

		Math::Vectorf3 GetUp() const
		{
			return m_rotation.GetUp();
		}

		void SetForward(const Vectorf3& newForward)
		{
			m_rotation.SetForward(newForward);
		}

		void SetRight(const Vectorf3& newRight)
		{
			m_rotation.SetForward(newRight);
		}

		void SetUp(const Vectorf3& newUp)
		{
			m_rotation.SetForward(newUp);
		}

		// Scale
		void SetScale(const Vectorf3& scale)
		{
			m_scale = scale;
		}

		Vectorf3 GetScale() const
		{
			return m_scale;
		}

		Transformf3D(const Transformf3D&) = default;
		Transformf3D(Transformf3D&&) = default;
		Transformf3D& operator=(const Transformf3D&) = default;
		Transformf3D& operator=(Transformf3D&&) = default;
		virtual ~Transformf3D() = default;

	private:
		Math::Vectorf3 m_position;
		Math::Vectorf3 m_scale;
		Math::Rotation m_rotation;
		
		Matrix4x4f m_orthoNormalBasis;
	};
}

#endif