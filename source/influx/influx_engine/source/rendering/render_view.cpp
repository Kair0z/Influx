#include "engine_pch.h"
#include "render_view.h"

namespace influx::engine
{
	render_view::render_view(const renderer::target_create_args& create_args)
	{
		m_target = renderer::create_target(create_args);
	}

	render_view::~render_view()
	{
		delete m_target;
	}

	const renderer::target& render_view::get_target() const
	{
		return *m_target;
	}

	renderer::world& render_view::get_renderworld()
	{
		return m_world;
	}

	renderer::worldview& render_view::get_renderworld_view()
	{
		return m_worldview;
	}

	math::transform3D& render_view::get_camera_transform()
	{
		return m_camera_transform;
	}
	influx::camera& render_view::get_camera_settings()
	{
		return m_camera_settings;
	}
	math::float4& render_view::get_clear_colour()
	{
		return m_clear_colour;
	}
}