#pragma once

// influx::engine
#include "influx_engine/common.h"
#include "influx_engine/config.h"

// influx::core
#include "core/singleton.h"
#include "core/threading/thread.h"
#include "core/file.h"

// influx::engine
#include "file/engine_files.h"

// influx::platform
namespace influx::platform
{
	class window;
	class window_event;
}

namespace influx::engine
{
	class render_manager;
	class content_manager;
	class editor_manager;
	class world;

	class engine final : public singleton<engine>
	{
	public:
		enum class run_type
		{
			game,
			editor,
			count
		};

		void run(run_type);

		platform::window const* get_window() const;

		content_manager* get_content() const;

		render_manager const* get_renderer() const;

		const frame_time& get_time() const;

		world* get_world() const;

		float get_fps() const;

		bool is_quit() const;

	private:
		void initialize();
		void cleanup();
		void run_input();
		void initialize_renderer(const string& window_name, const app_config& config);
		void poll_platform_events();
		
		void on_window_event(const platform::window_event& ev);

		bool m_is_quit_requested = false;

		engine_config m_config;
		app_config m_app_config;
		run_type m_runtype;
		thread m_inputthread;

		time::point m_t_init;
		time::point m_t_start;
		float m_fps;

		platform::window* m_window = nullptr;
		content_manager* m_contentman = nullptr;
		render_manager* m_renderman = nullptr;
		editor_manager* m_editorman = nullptr;
		world* m_world = nullptr;
		frame_time m_time{};

		bool m_is_quit = false;
	};

	inline static engine* get_engine()
	{
		return &engine::get_instance();
	}
}