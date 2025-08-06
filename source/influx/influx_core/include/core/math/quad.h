#pragma once

#include "core/math/Math.h"
#include "core/math/plane.h"
#include "core/math/Rect.h"

namespace influx::math
{
	// a quad is a 2D rectangle on a 3D plane
	template <typename _t>
	class quad final
	{
		using vector3 = math::vector<_t, 3u>;
		using vector2 = math::vector<_t, 2u>;
		using rect = rect<_t>;
		using plane = plane<_t>;

	public:
		quad(const planef& plane, const rectf& rect, const vector3& up = vector3::up())
			: m_plane{ plane }, m_up{ up }, m_rect{rect} {}

		static quad up_quad(const rectf& rect, const vector3& up = -vector3::forward(), float height = 0.0f);

		vector3 get_point(rect::e_point point) const;
		vector3 get_right() const;
		vector3 get_up() const;
		vector3 get_forward() const;

	private:
		plane m_plane{};
		rect m_rect{};
		vector3 m_up{};
	};

	using quadf = quad<float>;

	template<typename _t>
	inline quad<_t> quad<_t>::up_quad(const rectf& rect, const vector3& up, float height)
	{
		quad result = { plane(vector3::up(), height), rect };
		result.m_up = up;
		return result;
	}

	template <typename _t>
	inline quad<_t>::vector3 quad<_t>::get_point(rect::e_point point) const
	{
		vector3 origin = m_plane.get_origin();
		if (point == rect::e_point::mid)
		{
			return origin;
		}

		vector3 right = get_right();
		vector3 up = get_up();
		vector2 half_dimensions = m_rect.get_half_dimensions();

		switch (point)
		{
		case rect::e_point::left_bottom:	return origin + (-right * half_dimensions.x) + (-up * half_dimensions.y);
		case rect::e_point::left_top:		return origin + (-right * half_dimensions.x) + (up * half_dimensions.y);
		case rect::e_point::right_top:		return origin + (right * half_dimensions.x) + (up * half_dimensions.y);
		case rect::e_point::right_bottom:	return origin + (right * half_dimensions.x) + (-up * half_dimensions.y);
		}

		return {};
	}

	template<typename _t>
	inline quad<_t>::vector3 quad<_t>::get_right() const
	{
		return vector3::cross(get_forward(), get_up());
	}

	template<typename _t>
	inline quad<_t>::vector3 quad<_t>::get_up() const
	{
		return m_up;
	}

	template<typename _t>
	inline quad<_t>::vector3 quad<_t>::get_forward() const
	{
		return m_plane.m_normal;
	}
}