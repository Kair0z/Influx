#pragma once

// influx::engine
#include "render_common.h"

// influx::renderer
#include "influx_renderer.h"
namespace influx::renderer
{
	class target;
	struct target_create_args;
}

// influx::core
#include "core/string.h"
#include "core/math/transform.h"

namespace influx::engine
{
	/*
	*	a render view is a single render target
	*/
	class render_view final
	{
		static constexpr uint32 k_minimum_width = 64u;
		static constexpr uint32 k_minimum_height = 64u;

		renderer::target* m_target{};

		renderer::scene		m_scene{};
		renderer::scene2D	m_scene2D{};
		
		renderer::camera m_camera{};
		math::transform3D m_camera_transform{};

		math::float4 m_clear_colour = { 0,0,0,1 };

		math::uint2 m_dimensions = { 64u, 64u };
		math::uint2 m_prev_dimensions{};

		uint64 m_frame_counter = 0u;
		e_render_flags m_flags = e_render_flags::all;
		bool m_should_render = false;
		friend class render_manager;

	public:
		render_view() = default;
		render_view(const renderer::target_create_args& create_args);
		~render_view();

		const renderer::target& get_target() const;

		renderer::scene& get_scene();

		renderer::scene2D& get_scene2D();

		math::transform3D& get_camera_transform();
		
		renderer::camera& get_camera();

		math::float4& get_clear_colour();

		inline void clear_scenes()
		{

		}

		inline bool is_valid() const
		{
			return m_target != nullptr;
		}

		e_render_flags get_render_flags() const { return m_flags; }

		inline void set_dimensions(const math::uint2& dimensions)
		{
			m_dimensions = dimensions;
		}

		inline bool has_valid_dimensions() const
		{
			return m_dimensions.x >= k_minimum_width && m_dimensions.y >= k_minimum_height;
		}

		inline void set_render_enabled(bool enabled)
		{
			m_should_render = enabled;
		}

		inline bool should_render() const
		{
			return m_should_render;
		}
	};
}