#pragma once

// influx::core
#include "core/string.h"
#include "core/math/transform.h"

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

	private:
		string m_mesh_filepath;
	};
}