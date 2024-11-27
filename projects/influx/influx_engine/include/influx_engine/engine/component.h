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
		const math::transform3D& get_transform() const;
		math::transform3D& get_transform();

	private:
		transform_component() = default;
		transform_component(const math::transform3D & transform)
			: m_transform{ transform } {}

		math::transform3D m_transform;
	};

	class sprite_component final : public component
	{
	public:
		void set_texture_path(const string&);

	private:
		sprite_component() = default;
		string m_texture_filepath;
	};
}