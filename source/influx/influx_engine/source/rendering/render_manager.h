#pragma once

// influx::core
#include "core/macros.h"
#include "core/math/vector.h"
#include "core/enum.h"

// influx::engine
#include "imgui/imgui_manager.h"
#include "rendering/render_streamer.h"

// influx::renderer
#include "influx_renderer/scene.h"
namespace influx::renderer
{
	class target;
	struct target_create_args;
}

namespace influx::engine
{
	class engine;
	class content_manager;

	enum class e_render_flags : uint8
	{
		none			= 0,
		render_debug	= 1 << 0,
		render_scene	= 1 << 1,
		render_imgui	= 1 << 2,
		all				= render_debug | render_scene | render_imgui
	};

	enum class e_view_visibility_flags : uint8
	{
		none = 0,
		editor = 1 << 0,
		game = 1 << 1,
		all = game | editor
	};

	// a view contains a renderer::target to render to and data it wants rendered
	using render_view_id = string;
	class render_view final
	{
	public:
		render_view() = default;
		render_view(const renderer::target_create_args& create_args);
		~render_view();

		const renderer::target& get_target() const;

		renderer::scene& get_scene();
		renderer::scene2D& get_scene2D();

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
			return m_dimensions.x >= 64u && m_dimensions.y >= 64u;
		}

	private:
		renderer::target*	m_target{};
		renderer::scene		m_scene{};
		renderer::scene2D	m_scene2D{};
		math::float4 m_clear_colour = {};

		math::uint2 m_dimensions = { 64u, 64u };
		math::uint2 m_prev_dimensions{};

		uint64 m_frame_counter = 0u;
		e_render_flags m_flags = e_render_flags::all;
		friend class render_manager;
	};

	class render_manager final
	{
	public:
		/* main views supported by this engine */
		enum class e_render_view : uint8
		{
			scene_editor,
			game,
			count
		};
		static constexpr uint8 k_num_render_views = static_cast<uint8>(e_render_view::count);
		constexpr static const char* k_render_view_names[k_num_render_views]
		{
			"scene_editor",
			"game"
		};

	public:
		render_manager(engine* engine);
		~render_manager();

		void render();

		// takes care of backbuffer resizing
		void on_window_resize(const math::vectoru2& new_dimensions);

		// funnels assets from content_manager into the influx::renderer
		void stream_content(const content_manager& cont_man);

		void* get_loaded_texture_id(const string& name) const;

		bool is_debug_render_enabled() const;
		bool is_imgui_render_enabled() const;
		bool is_scene_render_enabled() const;

		// get imgui scene
		inline renderer::scene_imgui& get_imgui_scene() { return m_imgui_scene; }
		
		/* gets or (re)creates a target view that can be rendered to */
		render_view& get_renderview(e_render_view view);
		render_view& get_renderview(const render_view_id& id, const math::vectoru2& size);

	private:
		renderer::scene_imgui m_imgui_scene;
		imgui_manager m_imgui;
		render_streamer m_streamer{};
		umap<render_view_id, render_view> m_views{};
	};
}

ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_render_flags);
ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_view_visibility_flags);