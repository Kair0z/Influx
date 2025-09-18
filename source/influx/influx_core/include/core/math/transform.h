#pragma once

#ifndef __CORE_MATH_TRANSFORM_H_
#define __CORE_MATH_TRANSFORM_H_

#include "core/math/math.h"
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

		void set_position(const vectorf2& position)
		{
			m_position = position;
		}

		const vectorf2& get_position() const
		{
			return m_position;
		}

		transform2D(const transform2D&) = default;
		transform2D(transform2D&&) = default;
		transform2D& operator=(const transform2D&) = default;
		transform2D& operator=(transform2D&&) = default;
		virtual ~transform2D() = default;

	private:
		vectorf2 m_position = vectorf2::make_zero();
		vectorf2 m_scale = vectorf2::make_one();
		float m_rotation = 0.0f;
	};

	class transform3D final
	{
		vectorf3 m_position			= vectorf3::make_zero();
		vectorf3 m_scale			= vectorf3::make_one();
		math::rotation m_rotation	= rotation::identity();
		matrix4x4f m_matrix			= matrix4x4f::identity();
		bool m_is_matrix_dirty		= false;
		bool m_is_components_dirty	= false;

	public:
		transform3D() = default;
		transform3D(const math::vectorf3& position, const math::rotation& rotation = math::rotation::identity(), const math::vectorf3& scale = math::vectorf3::make_one())
			: m_position{ position }, m_rotation{ rotation }, m_scale{ scale }
		{
			m_is_matrix_dirty = true;
			update_matrix();
		}
		
		transform3D& operator=(const math::matrix4x4f& matrix)
		{
			set_matrix(matrix);
			return *this;
		}

		const static transform3D identity()
		{
			const static transform3D identity{ math::vectorf3::make_zero(), math::rotation::identity(), math::vectorf3::make_one()};
			return identity;
		}

		void translate(const vectorf3& add_position, bool blocal = false)
		{
			m_is_matrix_dirty = true;
			if (blocal)
			{
				vectorf3 delta =
					(get_right() * add_position.x) +
					(get_up() * add_position.y) +
					(get_forward() * add_position.z);

				set_position(get_position() + delta);
			}
			else
			{
				set_position(get_position() + add_position);
			}
		}

		void move(const vectorf3& delta_pos, bool blocal = false)
		{
			translate(delta_pos, blocal);
		}

		void rotate(float x, float y, float z, bool blocal = true)
		{
			m_is_matrix_dirty = true;
			if (blocal)
			{
				rotate(x, get_right());
				rotate(y, get_up());
				rotate(z, get_forward());
			}
			else
			{
				rotate(x, math::vectorf3::make_right());
				rotate(y, math::vectorf3::make_up());
				rotate(z, math::vectorf3::make_forward());
			}
		}

		void rotate(float delta_angle, const vectorf3& axis)
		{
			m_is_matrix_dirty = true;
			// m_rotation.rotate(delta_angle, axis);
		}

		void rotate_y(float delta_angle, bool blocal = false)
		{
			m_is_matrix_dirty = true;
			if (blocal)
			{
				rotate(delta_angle, get_up());
			}
			else
			{
				rotate(delta_angle, vectorf3::make_up());
			}
		}

		void rotate_x(float delta_angle, bool blocal = false)
		{
			m_is_matrix_dirty = true;

			if (blocal)
			{
				rotate(delta_angle, get_right());
			}
			else
			{
				rotate(delta_angle, vectorf3::make_right());
			}
		}

		void set_position(const vectorf3& position)
		{
			m_is_matrix_dirty = true;
			m_position = position;
		}

		void set_position(float x, float y, float z)
		{
			m_is_matrix_dirty = true;
			set_position({ x,y,z });
		}

		void set_position_x(float x)
		{
			m_is_matrix_dirty = true;
			m_position.x = x;
		}

		void set_position_y(float y)
		{
			m_is_matrix_dirty = true;
			m_position.y = y;
		}

		void set_position_z(float z)
		{
			m_is_matrix_dirty = true;
			m_position.z = z;
		}

		void set_forward(const vectorf3& newForward)
		{
			m_is_matrix_dirty = true;
			m_rotation.set_forward(newForward);
		}

		void set_right(const vectorf3& newRight)
		{
			m_is_matrix_dirty = true;
			m_rotation.set_right(newRight);
		}

		void set_up(const vectorf3& newUp)
		{
			m_is_matrix_dirty = true;
			m_rotation.set_up(newUp);
		}

		void set_rotation(const math::matrix4x4f& matrix)
		{
			m_is_matrix_dirty = true;
			m_rotation.set_rotation_matrix(matrix);
		}

		void set_scale(const vectorf3& scale)
		{
			m_is_matrix_dirty = true;
			m_scale = scale;
		}

		void set_scale(const float scale)
		{
			m_is_matrix_dirty = true;
			m_scale = vectorf3{ scale, scale, scale };
		}

		void look_at(const vectorf3& location)
		{
			m_is_matrix_dirty = true;
			const float3& normalized_fwd = (location - get_position()).normalized();
			set_forward(normalized_fwd);
			update_matrix();
		}

		const rotation& get_rotation() const
		{
			return m_rotation;
		}

		math::float3 get_rotation_eulers() const
		{
			return m_rotation.get_euler_angles();
		}

		vectorf3 get_position() const
		{
			return m_position;
		}

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

		vectorf3 get_scale() const
		{
			return m_scale;
		}

		matrix4x4f get_matrix() const
		{
			return m_matrix;
		}

		void set_matrix(const math::matrix4x4f& matrix)
		{
			m_matrix = matrix;
			m_is_components_dirty = true;
			update_components();
		}

		void update_matrix()
		{
			if (m_is_matrix_dirty)
			{
				m_matrix = matrix4x4f::make_transform_RH(m_position,
					m_rotation.get_forward(), m_rotation.get_up(),
					m_scale);
			}
			m_is_matrix_dirty = false;
		}

		void update_components()
		{
			if (m_is_components_dirty)
			{
				math::matrix4x4f out_rotation_mat{};
				m_matrix.decompose(m_position, out_rotation_mat, m_scale);
				m_rotation.set_rotation_matrix(out_rotation_mat);
			}
			m_is_components_dirty = false;
		}

		bool is_gimbal_locked() const
		{
			return false;
			//return m_rotation.is_gimbal_locked();
		}

		vectorf3 get_euler_angles() const
		{
			return m_rotation.get_euler_angles();
		}

		float get_pitch() const
		{
			return m_rotation.get_pitch_degrees();
		}

		float get_yaw() const
		{
			return m_rotation.get_yaw_degrees();
		}

		float get_roll() const
		{
			return m_rotation.get_roll_degrees();
		}

		transform3D(const transform3D&) = default;
		transform3D(transform3D&&) = default;
		transform3D& operator=(const transform3D&) = default;
		transform3D& operator=(transform3D&&) = default;
		virtual ~transform3D() = default;
	};
}

#endif