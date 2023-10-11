#pragma once

#ifndef __CORE_MATH_TRANSFORM_H_
#define __CORE_MATH_TRANSFORM_H_

#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/quaternion.h"

namespace influx::math
{
	class transform2D final
	{
	public:
		transform2D() = default;
		transform2D(const math::vectorf2& position, float rotation, const math::vectorf2& scale)
			: m_position{ position }, m_rotation{ rotation }, m_scale{ scale } {}

		transform2D(const transform2D&) = default;
		transform2D(transform2D&&) = default;
		transform2D& operator=(const transform2D&) = default;
		transform2D& operator=(transform2D&&) = default;
		virtual ~transform2D() = default;

	private:
		math::vectorf2 m_position;
		math::vectorf2 m_scale;
		float m_rotation;
	};

	class transform3D final
	{
	public:
		transform3D() = default;
		transform3D(const math::vectorf3& position, const math::rotation& rotation, const math::vectorf3& scale)
			: m_position{ position }, m_rotation{ rotation }, m_scale { scale } {}
		
		const static transform3D identity()
		{
			const static transform3D identity{ math::vectorf3::zero(), math::rotation::identity(), math::vectorf3::one()};
			return identity;
		}

		// Position
		void SetPosition(const vectorf3& position)
		{
			m_position = position;
		}

		vectorf3 get_position() const
		{
			return m_position;
		}
		
		// Rotation
		vectorf3 get_forward() const
		{
			return m_rotation.get_forward();
		}

		vectorf3 get_right() const
		{
			return m_rotation.get_right();
		}

		vectorf3 get_up() const
		{
			return m_rotation.get_up();
		}

		void set_forward(const vectorf3& newForward)
		{
			m_rotation.set_forward(newForward);
		}

		void set_right(const vectorf3& newRight)
		{
			m_rotation.set_forward(newRight);
		}

		void set_up(const vectorf3& newUp)
		{
			m_rotation.set_forward(newUp);
		}

		// Scale
		void set_scale(const vectorf3& scale)
		{
			m_scale = scale;
		}

		vectorf3 get_scale() const
		{
			return m_scale;
		}

		transform3D(const transform3D&) = default;
		transform3D(transform3D&&) = default;
		transform3D& operator=(const transform3D&) = default;
		transform3D& operator=(transform3D&&) = default;
		virtual ~transform3D() = default;

	private:
		vectorf3 m_position;
		vectorf3 m_scale;
		math::rotation m_rotation;
		
		matrix4x4f m_orthoNormalBasis;
	};
}

#endif