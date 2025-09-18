#include "engine_pch.h"

// influx::platform
#include "influx_platform/platform.h"
#include "influx_platform/window.h"

// influx::async
#include "influx_async.h"

// influx::input
#include "influx_input.h"

// influx::renderer
#include "influx_renderer/scene.h"

// influx::engine
#include "content/content_manager.h"
#include "rendering/render_manager.h"
#include "editor/editor_manager.h"
#include "game/game_manager.h"
#include "world/world.h"
#include "input/input_manager.h"
#include "tasks/task_manager.h"
#include "window/window_manager.h"
#include "engine_files.h"
#include "log/log_manager.h"
#include "scene/scene.h"
#include "project/project.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"

// tom++
#include "toml.h"

namespace influx::engine
{
	result<> engine::process_runarguments(int argc, char* argv[])
	{
		for (int i = 0u; i < argc; ++i)
		{
			const string& argument = argv[i];
			if (str::contains(argument, ".exe"))
			{
				// loading the editor by running the .exe
				m_parsed_run_args["exe_dir"] = to_string(path(argument).get_directory());
				m_parsed_run_args["exe"] = argument;
			}
			if (str::contains(argument, ".flx"))
			{
				// loading an .flx project
				m_parsed_run_args["projectfile"] = argument;

				// load the .flx project file
				auto load_project = project::load(argument);
				if (!load_project) return result<>::make_error("failed loading project at given filepath!");

				m_project = load_project.get();
			}

			m_run_args.push_back(argv[i]);
		}
		return {};
	}

	void engine::initialize()
	{
		m_t_init = time::get_now();
		m_logman = new log_manager();

		random::seed_random(0u);

		/* setup root file structure */
		m_config.m_file_influx_root		= get_engine_directory(engine_directory::root);
		m_config.m_file_influx_assets	= get_engine_directory(engine_directory::assets);
		m_config.m_file_influx_staged	= get_engine_directory(engine_directory::staged);

		/* make misc managers (order matters here :) )*/
		m_taskman		= new task_manager();
		m_inputman		= new input_manager();
		m_contentman	= new content_manager();
		m_gameman		= new game_manager();

		/* create window& renderer */
		const string window_name = (m_runtype == run_type::editor) ? "influx_editor" : "influx_game";
		const math::vectoru2 window_dimensions = { 720, 480u };
		platform::window_style window_style = platform::window_style::get_nodecoration();
		platform::window_desc window_desc{};
		const auto monitors = platform::monitor::query_monitors();
		
		window_desc	.set_dimensions(window_dimensions)
					.set_name(window_name)
					.set_style(window_style);
		m_windowman = new window_manager();
		m_windowman->spawn(window_desc); // main window
		m_renderman = new render_manager();

		// set to selected monitor
		const math::vectoru2 monitor_center = monitors[2].get_rect().get_mid();
		m_windowman->get_main_window().set_position(monitor_center);

		/* create world & scene_manager */
		m_world		= new world();
		m_sceneman	= new scene_manager();

		/* if editor, make editor_manager */
		if (m_runtype == run_type::editor)
		{
			m_editorman = new editor::editor_manager();
		}
	}

	result<> engine::run(run_type type, int argc, char* argv[])
	{
		m_runtype = type;

		auto process_run_args = process_runarguments(argc, argv);
		if (!process_run_args) return process_run_args;

		initialize();

		// if runtype is game, we immediately auto-start the game-manager
		if (type == run_type::game)
		{
			if (m_gameman)
				m_gameman->start();
		}

		while (!m_is_quit_requested)
		{
			influx_scope("frame");

			m_time.tick();
			m_fps = 1.0f / m_time.get_delta_seconds();

			// platform window event poll
			{
				influx_scope("poll_window");
				poll_platform_events();
				if (m_is_quit_requested) break;
			}
			// stream assets from content -> renderer
			{
				influx_scope("stream_to_render");
				m_renderman->stream_content(*m_contentman);
			}
			// input tick
			{
				influx_scope("input");
				m_inputman->tick();
			}
			// main update
			{
				influx_scope("update");
				if (m_gameman) m_gameman->tick();
				m_world->update();
			}
			// record imgui if editor
			if (m_runtype == run_type::editor)
			{
				m_renderman->get_imgui_scene().m_imgui_stacks.push_back([this](ImGuiContext& ctx)
				{
					m_editorman->on_imgui(ctx);
				});
			}
			// world builds renderscenes (pre-render)
			{
				influx_scope("pre_render");
				m_world->build_renderviews();
			}
			// render
			{
				influx_scope("render");
				m_renderman->render();
			}
			// log tick
			{
				influx_scope("log");
				m_logman->tick();
			}
		}

		if (type == run_type::game)
		{
			if (m_gameman) m_gameman->end();
		}

#if INFLUX_DEBUG
		influx::log_scopedata();
#endif

		cleanup();
		m_is_quit = true;
		return {};
	}

	void engine::cleanup()
	{
		async::shutdown();

		if (m_gameman)
		{
			delete m_gameman;
			m_gameman = nullptr;
		}
		if (m_editorman)
		{
			delete m_editorman;
			m_editorman = nullptr;
		}
		if (m_renderman)
		{
			delete m_renderman;
			m_renderman = nullptr;
		}
		if (m_windowman)
		{
			delete m_windowman;
			m_windowman = nullptr;
		}
		if (m_contentman)
		{
			delete m_contentman;
			m_contentman = nullptr;
		}
		if (m_inputman)
		{
			delete m_inputman;
			m_inputman = nullptr;
		}
		if (m_taskman)
		{
			delete m_taskman;
			m_taskman = nullptr;
		}
		if (m_world)
		{
			delete m_world;
			m_world = nullptr;
		}
		if (m_logman)
		{
			delete m_logman;
			m_logman = nullptr;
		}
	}

	void engine::poll_platform_events()
	{
		window_manager::poll_result result = m_windowman->poll_all();
		m_is_quit_requested = result.m_is_quited;
		m_is_quit_requested |= m_windowman->get_main_window().has_quit_request();
	}

	render_manager& engine::get_renderer()
	{
		return *m_renderman;
	}

	input_manager& engine::get_input()
	{
		return *m_inputman;
	}

	window_manager& engine::get_windowman()
	{
		return *m_windowman;
	}

	project& engine::get_current_project()
	{
		return get_engine()->m_project;
	}

	scene_manager& engine::get_sceneman()
	{
		return *get_engine()->m_sceneman;
	}

	scene& engine::get_current_scene()
	{
		return get_sceneman().get_current_scene();
	}

	log_manager& engine::get_logman()
	{
		return *get_engine()->m_logman;
	}

	content_manager& engine::get_content()
	{
		return *m_contentman;
	}

	game_manager& engine::get_game()
	{
		return *m_gameman;
	}

	world& engine::get_world()
	{
		return *m_world;
	}

	platform::window& engine::get_window()
	{
		return get_windowman().get_main_window();
	}

	editor::editor_manager& engine::get_editor()
	{
		return *m_editorman;
	}

	const frame_time& engine::get_time() const
	{
		return m_time;
	}

	float engine::get_fps() const
	{
		return m_fps;
	}
	
	bool engine::is_quit() const
	{
		return m_is_quit;
	}

	bool engine::is_editor() const
	{
		return m_runtype == run_type::editor;
	}

	bool engine::is_game() const
	{
		return m_runtype == run_type::game;
	}

	string engine::get_run_argument(const string& title)
	{
		if (get_engine()->m_parsed_run_args.contains(title))
		{
			return get_engine()->m_parsed_run_args[title];
		}
		return "";
	}
}

#include "influx_engine.h"
namespace influx::engine
{
	result<> run_editor(int argc, char* argv[])
	{
		return get_engine()->run(engine::run_type::editor, argc, argv);
	}

	result<> run_game(int argc, char* argv[])
	{
		return get_engine()->run(engine::run_type::game, argc, argv);
	}
}