#pragma once
#include <variant>
#include "core/math/colour.h"

#ifndef __CORE_SCENE_LIGHT_H_
#define __CORE_SCENE_LIGHT_H_

namespace influx::scene
{
	enum class e_light_type
	{
		directional,
		point,
		spot,
		count
	};

	class light final
	{
	public:
		struct directional
		{

		};

		struct point
		{
			float m_radius;
		};

		struct spot
		{
			float m_inner_angle;
			float m_outer_angle;
		};

		void set_type(e_light_type type)
		{
			m_type = type;
		}

		e_light_type get_type() const
		{
			return m_type;
		}

		void set_colour(const math::colour_rgba& colour)
		{
			m_colour = colour;
		}

		math::colour_rgba get_colour() const
		{
			return m_colour;
		}

		void set_inner_angle(float inner_angle)
		{
			m_spot.m_inner_angle = inner_angle;
		}

		void set_outer_angle(float outer_angle)
		{
			m_spot.m_outer_angle = outer_angle;
		}

		float get_inner_angle() const
		{
			return m_spot.m_inner_angle;
		}

		float get_outer_angle() const
		{
			return m_spot.m_outer_angle;
		}

		void set_attenuation(float att)
		{
			m_attenuation = att;
		}

		float get_attenuation() const
		{
			return m_attenuation;
		}

	private:
		e_light_type m_type;
		math::colour_rgba m_colour;
		float m_attenuation;

		directional m_directional;
		point		m_point;
		spot		m_spot;
	};
}

#endif