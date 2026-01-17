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
namespace influx::renderer
{
	template <e_texture_type _t>
	class texture;
}

namespace influx::engine
{
	class engine;
	class asset_manager;

	using render_cubemap = renderer::cubemap;
	using render_texture2D = renderer::texture2D;
	using render_texture3D = renderer::texture3D;

	/*
		engine level manager of the renderer
		initializes and keeps track of influx_renderer global state
	*/
	class render_manager final
	{
	public:
		render_manager();
		~render_manager();

		void render();

		// takes care of backbuffer resizing
		void on_window_resize(const math::vectoru2& new_dimensions);

		// funnels assets from content_manager into the influx::renderer
		void stream_content(const asset_manager& cont_man);

		bool is_debug_render_enabled() const;
		bool is_imgui_render_enabled() const;
		bool is_scene_render_enabled() const;

		// get imgui scene
		inline renderer::scene_imgui& get_imgui_scene() { return m_imgui_scene; }
		
		/* gets or (re)creates a target view that can be rendered to */
		render_view& get_renderview(e_render_view view);
		render_view& get_renderview(const render_view_id& id, const math::vectoru2& size);

		bool has_texture(const string& name) const;
		result<cptr<render_texture2D>> get_texture2D(const string& name) const;

	private:
		renderer::scene_imgui m_imgui_scene;
		imgui_manager m_imgui;
		render_streamer m_streamer{};
		umap<render_view_id, render_view> m_views{};
	};
}