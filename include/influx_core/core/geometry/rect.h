#pragma once

#ifndef _CORE_GEOMETRY_RECT_H_
#define _CORE_GEOMETRY_RECT_H_

#include "core/math/vector.h"
#include "core/math/Math.h"

namespace influx::math
{
	template <typename _t>
	struct rect final
	{
	private:
		using vector2 = math::vector<_t, 2u>;

	public:
		inline rect() = default;
		inline rect(_t l, _t b, _t w, _t h) : m_leftBottom{ l,b }, m_width_height{ w,h }{}
		inline rect(const vector2& lb, const vector2& wh) : m_leftBottom{ lb }, m_width_height{ wh }{}

		static rect square_rect(float radius);

		enum class e_point : uint8
		{
			left_bottom,
			left_top,
			right_top,
			right_bottom,
			mid,
			max
		};

		_t get_width() const
		{
			return m_width_height.x;
		}

		_t get_height() const
		{
			return m_width_height.y;
		}

		inline const vector2& get_dimensions() const
		{
			return m_width_height;
		}

		inline vector2 get_half_dimensions() const
		{
			return get_dimensions() * 0.5f;
		}

		const float get_aspect_ratio() const
		{
			return static_cast<float>(m_width_height.x) / static_cast<float>(m_width_height.y);
		}

		inline bool operator==(const rect& other) const
		{
			return m_leftBottom == other.m_leftBottom && m_width_height == other.m_width_height;
		}

		inline bool operator!=(const rect& other) const
		{
			return !(*this == other);
		}

		inline _t get_left() const
		{
			return m_leftBottom.x;
		}

		inline _t get_right() const
		{
			return m_leftBottom.x + m_width_height.x;
		}

		inline _t get_bottom() const
		{
			return m_leftBottom.y;
		}

		inline _t get_top() const
		{
			return m_leftBottom.y + m_width_height.y;
		}

		vector2 get_mid() const
		{
			return vector2(m_leftBottom.x + (m_width_height.x * 0.5f), m_leftBottom.y + (m_width_height.y * 0.5f));
		}

		const vector2& get_point(const e_point point) const
		{
			switch (point)
			{
			case e_point::left_bottom: return m_leftBottom;
			case e_point::left_top: return vector2{ get_left(), get_top()};
			case e_point::right_top: return vector2{ get_right(), get_top()};
			case e_point::right_bottom: return vector2{ get_right(), get_bottom()};
			case e_point::mid: return get_mid();
			}

			return {};
		}

		vector2 m_leftBottom{};
		vector2 m_width_height{};
	};

	using rectf = rect<float>;
	using rectu = rect<uint32>;
	using recti = rect<int>;
	using rectd = rect<double>;

	template<typename _t>
	inline rect<_t> rect<_t>::square_rect(float radius)
	{
		float radians = math::to_radians(45.0f);
		rect<_t> result{};
		result.m_leftBottom = {};
		result.m_width_height = vector2(
			2.0f * radius * math::cos(radians), 
			2.0f * radius * math::sin(radians));
		return result;
	}
}

#endif