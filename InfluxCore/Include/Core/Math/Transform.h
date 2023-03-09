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