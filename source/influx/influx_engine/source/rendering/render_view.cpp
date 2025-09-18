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

	renderer::scene& render_view::get_scene()
	{
		return m_scene;
	}

	renderer::scene2D& render_view::get_scene2D()
	{
		return m_scene2D;
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