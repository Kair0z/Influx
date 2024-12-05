#pragma once

// influx::core
#include "core/string.h"
#include "core/math/transform.h"
#include "core/scene/camera.h"

namespace influx::engine
{
	class component
	{
	protected:
		component() = default;
	};

	class transform_component final : public component
	{
	public:
		transform_component() = default;
		transform_component(const math::transform3D & transform)
			: m_transform{ transform } {}

		const math::transform3D& get_transform() const
		{
			return m_transform;
		}

		math::transform3D& get_transform()
		{
			return m_transform;
		}

	private:
		math::transform3D m_transform;
	};

	class sprite_component final : public component
	{
	public:
		sprite_component() = default;

		void set_texture_path(const string& path)
		{
			m_texture_filepath = path;
		}

		const string& get_texture_path() const
		{
			return m_texture_filepath;
		}

	private:
		string m_texture_filepath;
	};

	class mesh_component final : public component
	{
	public:
		mesh_component() = default;

		void set_mesh_path(const string& path)
		{
			m_mesh_filepath = path;
		}

		const string& get_mesh_path() const
		{
			return m_mesh_filepath;
		}

		void set_visible(bool new_vis)
		{
			m_is_visible = new_vis;
		}

		bool is_visible() const
		{
			return m_is_visible;
		}

	private:
		string m_mesh_filepath;
		bool m_is_visible;
	};

	class material_component final
	{
	public:
		void set_color(const math::vectorf4& color)
		{
			m_color = color;
		}

		const math::vectorf4& get_color() const
		{
			return m_color;
		}

	private:
		math::vectorf4 m_color;
	};

	class camera_component final : public component
	{
	public:
		void set_fov(float fov)
		{
			m_camera.set_fov(fov);
		}

		float get_fov() const
		{
			return m_camera.get_fov();
		}

	private:
		influx::scene::camera m_camera{};
	};
}