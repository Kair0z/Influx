#pragma once

// influx::core
#include "core/math/vector.h"

// influx::engine
#include "rendering/render_common.h"
#include "rendering/render_view.h"
#include "imgui/imgui_manager.h"
#include "rendering/render_streamer.h"

// influx::renderer
#include "influx_renderer/scene.h"

namespace influx::engine
{
	class engine;
	class content_manager;

	class render_manager final
	{
	public:
		render_manager(engine* engine);
		~render_manager();

		void render();

		// takes care of backbuffer resizing
		void on_window_resize(const math::vectoru2& new_dimensions);

		// funnels assets from content_manager into the influx::renderer
		void stream_content(const content_manager& cont_man);

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