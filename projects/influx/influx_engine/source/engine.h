#pragma once

// influx::engine
#include "common.h"
#include "config/config.h"

// influx::core
#include "core/singleton.h"
#include "core/threading/thread.h"
#include "core/file.h"
#include "core/result.h"
#include "core/pointer.h"

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
	class game_manager;
	class input_manager;
	class task_manager;
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

		content_manager& get_content();
		game_manager& get_game();
		world& get_world();
		platform::window& get_window();
		editor_manager& get_editor();
		render_manager& get_renderer();

		const frame_time& get_time() const;
		float get_fps() const;
		bool is_quit() const;

		bool is_editor() const;
		bool is_game() const;

	private:
		void initialize();
		void cleanup();

		void initialize_renderer(const string& window_name, const math::vectoru2& size);
		void poll_platform_events();
		
		void on_window_event(const platform::window_event& ev);

		bool m_is_quit_requested = false;

		engine_config m_config;
		app_config m_app_config;
		run_type m_runtype;
		thread m_inputthread;
		thread m_contentthread;

		time::point m_t_init;
		time::point m_t_start;
		float m_fps;

		platform::window* m_window = nullptr;
		content_manager* m_contentman = nullptr;
		render_manager* m_renderman = nullptr;
		editor_manager* m_editorman = nullptr;
		game_manager* m_gameman = nullptr;
		input_manager* m_inputman = nullptr;
		task_manager* m_taskman = nullptr;
		world* m_world = nullptr;
		frame_time m_time{};

		bool m_is_quit = false;
	};

	inline static engine* get_engine()
	{
		return &engine::get_instance();
	}
}