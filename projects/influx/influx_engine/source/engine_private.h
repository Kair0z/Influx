#pragma once

#include "engine/engine.h"

#include "core/singleton.h"
#include "core/threading/thread.h"

namespace influx::platform
{
	class window;
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

		void run();

		file get_engine_directory(e_directory dir);

		platform::window const* get_window() const;

		content_manager const* get_content() const;

		render_manager const* get_renderer() const;

		bool is_quit() const;

	private:
		void run_game();
		void run_editor();
		void initialize_windowed_renderer(const app_config& config);

		bool m_is_quit_requested = false;
		game_module* m_gamemodule = nullptr;
		editor_module* m_editormodule = nullptr;
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