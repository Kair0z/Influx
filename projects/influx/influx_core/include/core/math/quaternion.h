#pragma once

#ifndef __CORE_MATH_QUATERNION_H_
#define __CORE_MATH_QUATERNION_H_

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
			return m_forward;
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
			m_right = m_rotation_matrix * m_right;
			m_up = m_rotation_matrix * m_up;
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