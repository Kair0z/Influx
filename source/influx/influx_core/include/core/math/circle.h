#pragma once

#include "core/math/math.h"
#include "core/math/vector.h"

namespace influx::math
{
	template <typename _t>
	struct circle2D final
	{
	private:
		using vector2 = math::vector<_t, 2u>;

	public:
		inline circle2D()
			: m_radius{}, m_position{} {}

		inline circle2D(const vector2& position, const _t radius)
			: m_radius{ radius }, m_position{ position } {}

		void grow(const vector2& point);
		void shrink(const vector2& point);

		_t		m_radius;
		vector2 m_position;
	};

	template <typename _t>
	void circle2D<_t>::grow(const vector2& point)
	{
		const vector2 diff = (point - m_position);
		if (diff.is_zero())
		{
			return;
		}

		float distance = diff.magnitude();
		if (m_radius < distance)
		{
			m_radius = distance;
		}
	}

	template <typename _t>
	void circle2D<_t>::shrink(const vector2& point)
	{
		const vector2 diff = (point - m_position);
		if (diff.is_zero())
		{
			return;
		}

		float distance = diff.magnitude();
		if (m_radius > distance)
		{
			m_radius = distance;
		}
	}

	template <typename _t>
	struct circle3D final
	{
	private:
		using vector3 = math::vector<_t, 3u>;

	public:
		inline circle3D()
			: m_radius{}, m_position{}, m_normal{ vector3::up() } {}

		inline circle3D(const vector3& position, const vector3& normal, const _t radius)
			: m_radius{ radius }, m_position{ position }, m_normal{ normal } {}

		void grow(const vector3& point);
		void shrink(const vector3& point);

		vector3 get_point_at_degrees(float angle_degrees) const
		{
			return m_position + (get_normal_at_degrees(angle_degrees) * m_radius);
		}

		vector3 get_normal_at_degrees(float angle_degrees) const
		{
			float angle_radians = math::to_radians(angle_degrees);

			vector3 local_normal{};
			if (m_normal == vector3::up())
			{
				// default case:
				return { math::cos(angle_radians), 0.0f, math::sin(angle_radians) };
			}
			else
			{
				const vector3 vec_u = vector3::cross(m_normal, vector3::up());
				const vector3 vec_v = vector3::cross(vec_u, m_normal);
				local_normal = (vec_u * math::cos(angle_radians)) + (vec_v * math::sin(angle_radians));
			}

			return local_normal.normalized();
		}

		_t		m_radius;
		vector3 m_position;
		vector3 m_normal;
	};

	template <typename _t>
	void circle3D<_t>::grow(const vector3& point)
	{
		const vector3 diff = (point - m_position);
		if (diff.is_zero())
		{
			return;
		}

		float distance = diff.magnitude();
		if (m_radius < distance)
		{
			m_radius = distance;
		}
	}

	template <typename _t>
	void circle3D<_t>::shrink(const vector3& point)
	{
		const vector3 diff = (point - m_position);
		if (diff.is_zero())
		{
			return;
		}

		float distance = diff.magnitude();
		if (m_radius > distance)
		{
			m_radius = distance;
		}
	}

	using circlef3D = circle3D<float>;
}