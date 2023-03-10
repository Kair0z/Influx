#pragma once

#ifndef __CORE_MATH_QUATERNION_H_
#define __CORE_MATH_QUATERNION_H_

namespace Influx::Math
{
	class Quaternion final
	{
	public:
		Quaternion() = default;
		virtual ~Quaternion() = default;

		const static Quaternion Identity()
		{
			static Quaternion q{};
			return q;
		}

		Math::Vectorf3 GetForward() const
		{
			return m_forward;
		}

		Math::Vectorf3 GetRight() const
		{
			return m_right;
		}

		Math::Vectorf3 GetUp() const
		{
			return m_up;
		}

		void SetForward(const Vectorf3& newForward)
		{
			m_forward = newForward;
		}

		void SetRight(const Vectorf3& newRight)
		{
			m_right = newRight;
		}

		void SetUp(const Vectorf3& newUp)
		{
			m_up = newUp;
		}

	private:
		Math::Vectorf3 m_forward;
		Math::Vectorf3 m_right;
		Math::Vectorf3 m_up;
	};

	// Temp... Might make this a standalone-class someday..
	using Rotation = Quaternion;
}

#endif