#pragma once

#ifndef __CORE_MATH_QUATERNION_H_
#define __CORE_MATH_QUATERNION_H_

#include "core/math/math.h"
#include "core/math/vector.h"

namespace influx::math
{
	class quaternion final
	{
	public:
		quaternion() = default;
		quaternion(const vectorf3& forward, const vectorf3& up = vectorf3::up())
		{
			m_forward = forward.normalized();
			m_right = vectorf3::cross(up, m_forward);
			m_up = vectorf3::cross(m_right, m_forward);
		}

		virtual ~quaternion() = default;

		const static quaternion identity()
		{
			static quaternion q{};
			q.m_up = { 0.0f, 1.0f, 0.0f };
			q.m_forward = { 0.0f, 0.0f, 1.0f };
			q.m_right = { 1.0f, 0.0f, 0.0f };
			return q;
		}

		vectorf3 get_forward() const
		{
			return m_forward.normalized();
		}

		vectorf3 get_right() const
		{
			return m_right;
		}

		vectorf3 get_up() const
		{
			return m_up;
		}

		void set_forward(const vectorf3& newForward)
		{
			m_forward = newForward;
		}

		void set_right(const vectorf3& newRight)
		{
			m_right = newRight;
		}

		void set_up(const vectorf3& newUp)
		{
			m_up = newUp;
		}

		void rotate(float delta_angle, const vectorf3& axis)
		{
			m_rotation_matrix = math::matrix4x4f::make_rotation(axis, delta_angle);

			m_forward = m_rotation_matrix * m_forward;
			m_right = math::vectorf3::cross(m_forward, vectorf3::up());
			m_up = math::vectorf3::cross(m_right, m_forward);
		}

		bool is_gimbal_locked() const
		{
			return math::abs(m_forward[1]) > 0.9999;
		}

		vectorf3 get_euler_angles() const
		{
			return { 
				math::to_degrees(get_pitch()), 
				math::to_degrees(get_yaw()), 
				math::to_degrees(get_roll()) };
		}

		float get_pitch() const
		{
			return asinf(-m_forward.y);
		}

		float get_yaw() const
		{
			return atan2f(m_forward.x, m_forward.z);
		}

		float get_roll() const
		{
			return atan2f(m_right.y, m_right.x);
		}

	private:
		vectorf3 m_forward;
		vectorf3 m_right;
		vectorf3 m_up;

		math::matrix4x4f m_rotation_matrix;
	};

	using rotation = quaternion;
}

#endif