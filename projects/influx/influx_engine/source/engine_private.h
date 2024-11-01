#pragma once

// influx::engine
#include "influx_engine/engine/engine.h"

// influx::core
#include "core/singleton.h"
#include "core/threading/thread.h"

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

	class engine final : public singleton<engine>
	{
	public:
		enum class e_directory : uint8
		{
			root,
			assets,
			resources,
			staged,
			intermediate,
			binaries,
			count
		};

		enum class run_type
		{
			game,
			editor,
			count
		};

		void run(base_module* mod);

		file get_engine_directory(e_directory dir);

		platform::window const* get_window() const;

		content_manager const* get_content() const;

		render_manager const* get_renderer() const;

		bool is_quit() const;

	private:
		void initialize();
		void cleanup();
		void run_game();
		void run_editor();
		void run_input();
		void initialize_renderer(const string& window_name, const app_config& config);
		void poll_platform_events();
		
		void on_window_event(const platform::window_event& ev);

		bool m_is_quit_requested = false;

		base_module* m_module = nullptr;
		game_module* m_game = nullptr;
		editor_module* m_editor = nullptr;

		engine_config m_config;
		app_config m_app_config;
		run_type m_runtype;
		thread m_inputthread;

		time::point m_t_init;
		time::point m_t_start;

		platform::window* m_window = nullptr;
		content_manager* m_contentman = nullptr;
		render_manager* m_renderman = nullptr;

		bool m_is_quit = false;
	};

	inline static engine* get_engine()
	{
		return &engine::get_instance();
	}
}