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
		virtual ~quaternion() = default;

		const static quaternion identity()
		{
			static quaternion q{};
			return q;
		}

		vectorf3 GetForward() const
		{
			return m_forward;
		}

		vectorf3 GetRight() const
		{
			return m_right;
		}

		vectorf3 GetUp() const
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

	private:
		vectorf3 m_forward;
		vectorf3 m_right;
		vectorf3 m_up;
	};

	// Temp... Might make this a standalone-class someday..
	using Rotation = quaternion;
}

#endif