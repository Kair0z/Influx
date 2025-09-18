#pragma once

#include "PixelRenderer.h"
#include "core/math/ray.h"

namespace influx
{
	class pixel_raytracer : public pixel_renderer
	{
	public:
		pixel_raytracer() = default;

		virtual pixel_output render_pixel(const render_scene& scene, const math::vectorf2& uv, const float ar) const override;

	private:
		struct hit_record final
		{
			math::vectorf3 m_normal;
			math::vectorf3 m_world_position;
			math::vectorf3 m_to_view;
			float m_t;
			float m_dot;
		};

		math::ray create_viewray(const math::vectorf2& uv, const float ar, float sampleRandStrength) const;
		math::ray create_viewray(const math::vectorf2& uv, const float ar) const;
		bool trace_sphere(const math::sphere<float>& sphere, const math::ray& ray, hit_record& out_hitRecord, float& depthBuffer) const;
	};
}


