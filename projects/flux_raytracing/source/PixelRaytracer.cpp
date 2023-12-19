#include "PixelRaytracer.h"

#include "Core/Math/Math.h"
#include "Core/Math/Random.h"
#include "Core/BRDF/BRDF.h"

#include "PixEvents.h"

namespace influx
{
	pixel_renderer::pixel_output influx::pixel_raytracer::render_pixel(
		const render_scene& scene, const math::vectorf2& uv, const float ar) const
	{
		math::ray ray = create_viewray(uv, ar);

		float depth = get_render_settings().m_depth_min_max.y;
		bool anyHit = false;
		hit_record record{};

		// For each object:
		size_t sphereHitIdx = 0;
		for (size_t i = 0; i < scene.m_spheres.size(); ++i)
		{
			const math::sphere<float>& sphere = scene.m_spheres[i];

			if (trace_sphere(sphere, ray, record, depth))
			{
				anyHit = true;
				sphereHitIdx = i;
			}
		}

		pixel_output pixelResult{};
		pixelResult.m_depth = record.m_t;
		pixelResult.m_is_anything_rendered = anyHit;

		if (anyHit)
		{
			switch (get_render_mode())
			{
			default:
			case e_render_mode::material:
			{
				BRDF::phong_settings settings{};
				settings.m_phong_exponent = 100.0f;
				settings.m_specular = 0.7f;
				settings.m_diffuse_colour = { 0.8f, 0.4f, 0.7f };
				settings.m_diffuse = 0.6f;

				settings.m_diffuse_colour *= scene.m_randoms[sphereHitIdx];

				auto result = BRDF::phong(settings, scene.m_light.m_direction, scene.m_light.m_colour, 
					record.m_to_view, record.m_normal);

				pixelResult.m_rgba = { result.r, result.g, result.b, 1.0f };
				break;
			}

			case e_render_mode::normals:
			{
				math::vectorf3 colouredNormal = record.m_normal;
				colouredNormal += math::vectorf3{ 1.0f, 1.0f, 1.0f };
				colouredNormal *= 0.5f;

				pixelResult.m_rgba = { colouredNormal.r, colouredNormal.g, colouredNormal.b, 1.0f };
				break;
			}

			case e_render_mode::depth:
			{
				float colouredDepth = remap_depth(record.m_t);
				pixelResult.m_rgba = { colouredDepth, colouredDepth, colouredDepth, 1.0f };
				break;
			}

			}
		}

		return pixelResult;
	}

	math::ray pixel_raytracer::create_viewray(const math::vectorf2& uv, const float ar, float sampleRandStrength) const
	{
		math::vectorf3 worldOrigin = get_camera_position();
		math::vectorf3 worldDirection = get_camera_forward();

		math::vectorf2 ndc = (2 * uv) - math::vectorf2(1.0f, 1.0f); // [uv:0,1] => [ndc:-1,1]

		if (sampleRandStrength > 0.0f)
		{
			// Apply a random pixel-offset
			worldOrigin.x += (ndc.x + (sampleRandStrength * random::get_tent_random<float>() * ar)) * get_camera().get_fov() * ar;
			worldOrigin.y += (ndc.y + (sampleRandStrength * random::get_tent_random<float>())) * get_camera().get_fov();
		}
		else
		{
			worldOrigin.x += (ndc.x) * get_camera().get_fov() * ar;
			worldOrigin.y += (ndc.y) * get_camera().get_fov();
		}

		float min = get_render_settings().m_depth_min_max.x;
		float max = get_render_settings().m_depth_min_max.y;

		return math::ray(worldOrigin, worldDirection, min, max);
	}

	math::ray pixel_raytracer::create_viewray(const math::vectorf2& uv, const float ar) const
	{
		return create_viewray(uv, ar, 0.0f);
	}

	bool pixel_raytracer::trace_sphere(const math::sphere<float>& sphere, const math::ray& ray, pixel_raytracer::hit_record& out_hitRecord, float& depthBuffer) const
	{
		float a = math::vectorf3::dot(ray.get_direction(), ray.get_direction());
		float b = math::vectorf3::dot(2 * ray.get_direction(), ray.get_direction() - sphere.m_position);
		float c = math::vectorf3::dot(ray.get_origin() - sphere.m_position, ray.get_origin() - sphere.m_position) - (sphere.m_radius * sphere.m_radius);

		float d{ (b * b) - 4 * a * c };

		if (d <= 0.0f) return false;

		float dSqrt{ sqrtf(d) };

		float t1{ (-b - dSqrt) / 2.f * a };
		float t2;

		if (t1 < ray.get_minimum())
		{
			t2 = (-b + dSqrt) / 2.f * a;
		}

		if (t1 > ray.get_maximum() || t1 < ray.get_minimum()) return false;

		// Depth
		if (t1 > depthBuffer) return false;

		// Fill out hitrecord
		out_hitRecord.m_world_position = ray.get_origin() + ray.get_direction() * t1;
		out_hitRecord.m_normal = (out_hitRecord.m_world_position - sphere.m_position).normalized();
		out_hitRecord.m_dot = math::vectorf3::dot(ray.get_direction(), out_hitRecord.m_normal);
		out_hitRecord.m_t = t1;
		out_hitRecord.m_to_view = (ray.get_origin() - out_hitRecord.m_world_position).normalized();

		depthBuffer = t1;
		return true;
	}
}
