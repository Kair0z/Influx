#pragma once

#include "Core/Math/Math.h"
#include "Core/Scene/Camera.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Plane.h"

#include <vector>

namespace influx
{
	struct render_scene final
	{
		using sphere = influx::math::sphere<float>;
		std::vector<float> m_randoms;
		std::vector<sphere> m_spheres;

		struct direction_light final
		{
			math::vectorf3 m_direction;
			math::vectorf3 m_colour;
		};

		direction_light m_light;
	};

	class pixel_renderer
	{
	public:
		struct pixel_output final
		{
			math::vectorf4 m_rgba;
			float m_depth;
			bool m_is_anything_rendered;
		};

		enum class e_render_mode
		{
			material,
			depth,
			normals
		};

		struct render_settings final
		{
			e_render_mode m_mode{};
			math::vectorf2 m_depth_min_max = {0.0f, FLT_MAX};
		};

	public:
		virtual pixel_output render_pixel(const render_scene& scene, const math::vectorf2& uv, const float ar) const = 0;

		void set_camera_fov(const float newFov) { m_camera.set_fov(newFov); }
		void set_camera_position(const math::vectorf3& newPosition) { m_camera_position = newPosition; }
		void set_camera_forward(const math::vectorf3& newForward) { m_camera_forward = newForward; }
		void set_render_mode(const e_render_mode renderMode) { m_renderSettings.m_mode = renderMode; }

		const scene::camera& get_camera() const { return m_camera; }
		const math::vectorf3& get_camera_position() const { return m_camera_position; }
		const math::vectorf3& get_camera_forward() const { return m_camera_forward; }
		
		const e_render_mode get_render_mode() const { return m_renderSettings.m_mode; }

		render_settings& get_render_settings() { return m_renderSettings; }
		const render_settings& get_render_settings() const { return m_renderSettings; }

	protected:
		inline float remap_depth(float depth_value) const
		{
			return math::remap(depth_value,
				m_renderSettings.m_depth_min_max.x, m_renderSettings.m_depth_min_max.y,
				0.0f, 1.0f);
		}

	private:
		scene::camera m_camera;
		math::vectorf3 m_camera_position;
		math::vectorf3 m_camera_forward;
		render_settings m_renderSettings;

	public:
		pixel_renderer() = default;
		pixel_renderer(const pixel_renderer&) = delete;
		pixel_renderer(pixel_renderer&&) = delete;
		pixel_renderer& operator=(const pixel_renderer&) = delete;
		pixel_renderer& operator=(pixel_renderer&&) = delete;
		virtual ~pixel_renderer() = default;
	};
}


