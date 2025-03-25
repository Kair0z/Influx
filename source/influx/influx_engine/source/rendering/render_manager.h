#pragma once

// influx::core
#include "core/macros.h"
#include "core/math/vector.h"
#include "core/enum.h"

// influx::engine
#include "imgui/imgui_manager.h"
#include "rendering/render_streamer.h"

// influx::renderer
namespace influx::renderer
{
	class target;
	class scene;
	struct scene_imgui;
	struct scene2D;
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

	class render_manager final
	{
		INFLUX_NO_MOVE(render_manager);
		INFLUX_NO_COPY(render_manager);

	public:
		render_manager(engine* engine);
		~render_manager();

		void render(
			const renderer::scene& scene,
			const renderer::scene2D& scene2D,
			const renderer::scene_imgui& imgui);

		// takes care of backbuffer resizing
		void on_window_resize(const math::vectoru2& new_dimensions);

		// funnels assets from content_manager into the influx::renderer
		void stream_content(const content_manager& cont_man);

		void* get_loaded_texture_id(const string& name) const;

		bool is_debug_render_enabled() const;
		bool is_imgui_render_enabled() const;
		bool is_scene_render_enabled() const;

	private:
		e_render_flags m_render_flags{};
		imgui_manager m_imgui;
		render_streamer m_streamer{};
		renderer::target* mp_window_target;
		renderer::target* mp_scene_target;
	};
}

ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_render_flags);