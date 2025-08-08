#pragma once

// influx::platform
namespace influx::platform
{
	class window;
	class window_event;
}

// influx::engine
#include "common.h"
#include "config/config.h"
#include "log/log_manager.h"
#include "file/project.h"

namespace influx::engine
{
	namespace editor
	{
		class editor_manager;
	}
	class render_manager;
	class content_manager;
	class game_manager;
	class input_manager;
	class task_manager;
	class window_manager;
	class scene_manager;
	class scene;
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
		void run(run_type, int argc, char* argv[]);

		content_manager& get_content();
		game_manager& get_game();
		world& get_world();
		platform::window& get_window();
		editor::editor_manager& get_editor();
		render_manager& get_renderer();
		input_manager& get_input();
		window_manager& get_windowman();

		static project& get_current_project();
		static scene_manager& get_sceneman();
		static scene& get_current_scene();

		static log_manager& get_logman();

		const frame_time& get_time() const;
		float get_fps() const;
		bool is_quit() const;
		
		bool is_editor() const;
		bool is_game() const;

		static string get_run_argument(const string&);

	private:
		void process_runarguments(int argc, char* argv[]);
		void initialize();
		void cleanup();

		void poll_platform_events();
		
		void on_window_event(const platform::window_event& ev);

		bool m_is_quit_requested = false;

		engine_config m_config;
		app_config m_app_config;
		run_type m_runtype;
		thread m_inputthread;
		thread m_contentthread;
		vector<string> m_run_args;
		umap<string, string> m_parsed_run_args;
		project m_project;

		time::point m_t_init;
		time::point m_t_start;
		float m_fps;

		scene_manager* m_sceneman = nullptr;
		window_manager* m_windowman = nullptr;
		content_manager* m_contentman = nullptr;
		render_manager* m_renderman = nullptr;
		editor::editor_manager* m_editorman = nullptr;
		game_manager* m_gameman = nullptr;
		input_manager* m_inputman = nullptr;
		task_manager* m_taskman = nullptr;
		log_manager* m_logman = nullptr;
		world* m_world = nullptr;
		frame_time m_time{};

		bool m_is_quit = false;

	public:
		template <typename ..._args>
		static inline void log(e_log_category category, const string& format, const _args&... args)
		{
			get_logman().log(category, format, args...);
		}
		static void log(const char*);
	};

	inline static engine* get_engine()
	{
		return &engine::get_instance();
	}
}