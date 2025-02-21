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
#include "file/engine_files.h"
#include "log/log_manager.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"

namespace influx::engine
{
	void engine::process_runarguments(int argc, char* argv[])
	{
		for (int i = 0u; i < argc; ++i)
		{
			const string& argument = argv[i];

			if (str::contains(argument, ".exe"))
			{
				// loading the editor by running the .exe
				m_parsed_run_args["exe_dir"] = file(argument).m_directory;
				m_parsed_run_args["exe"] = argument;
			}
			if (str::contains(argument, ".flx"))
			{
				// loading an .flx project
				m_parsed_run_args["projectfile"] = argument;
			}

			m_run_args.push_back(argv[i]);
		}
	}

	void engine::initialize()
	{
		m_logman = new log_manager();

		random::seed_random(0u);

		m_t_init = time::get_now();

		m_config.m_file_influx_root = get_engine_directory(engine_directory::root);
		m_config.m_file_influx_assets = get_engine_directory(engine_directory::assets);
		m_config.m_file_influx_staged = get_engine_directory(engine_directory::staged);

		m_taskman = new task_manager();
		m_inputman = new input_manager();
		m_contentman = new content_manager(this);
		m_gameman = new game_manager();
		if (m_runtype == run_type::editor)
		{
			m_editorman = new editor::editor_manager();
		}

		// initialize render
		const string window_name = (m_runtype == run_type::editor) ? "influx_editor" : "influx_game";
		const math::vectoru2 window_dimensions = { 720, 480u};
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(window_dimensions)
			.set_name(window_name);

		m_windowman = new window_manager();
		m_windowman->spawn(window_desc); // main window

		m_renderman = new render_manager(this);
		m_world = new world();
	}

	void engine::run(run_type type, int argc, char* argv[])
	{
		m_runtype = type;
		
		process_runarguments(argc, argv);

		initialize();

		while (!m_is_quit_requested)
		{
			influx_scope("frame");

			m_time.tick();
			m_fps = 1.0f / m_time.get_delta_seconds();

			// platform window tick
			{
				influx_scope("poll_window");
				poll_platform_events();
				if (m_is_quit_requested) break;
			}
			// input tick
			{
				influx_scope("input");
				m_inputman->tick();
			}
			// main update
			{
				influx_scope("update");
				m_world->update();
			}
			// stream available assets from content into the renderer
			{
				influx_scope("stream_to_render");
				m_renderman->stream_content(*m_contentman);
			}
			
			// build a render-scene
			renderer::scene scene{};
			renderer::scene_debug debug{};
			renderer::scene2D scene2D{};
			renderer::scene_imgui imgui{};

			// record imgui if editor
			if (m_runtype == run_type::editor)
			{
				imgui.m_imgui_stacks.push_back([this](ImGuiContext& ctx)
				{
					m_editorman->on_imgui(ctx);
				});
			}

			// world builds renderscene 
			{
				influx_scope("build_render");
				scene.m_seconds = m_time.get_time_seconds();
				scene.m_delta_seconds = m_time.get_delta_seconds();
				m_world->build_renderscene(scene, scene2D, debug);
			}

			// render
			{
				influx_scope("render");
				m_renderman->render(scene, scene2D, imgui, debug);
			}

			// log tick
			{
				influx_scope("log");
				m_logman->tick();
			}
		}

#if INFLUX_DEBUG
		influx::log_scopedata();
#endif

		cleanup();
		m_is_quit = true;
	}

	void engine::cleanup()
	{
		async::shutdown();

		if (m_gameman)
		{
			delete m_gameman;
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
		}

		if (m_taskman)
		{
			delete m_taskman;
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
	void run_editor(int argc, char* argv[])
	{
		get_engine()->run(engine::run_type::editor, argc, argv);
	}

	void run_game(int argc, char* argv[])
	{
		get_engine()->run(engine::run_type::game, argc, argv);
	}
}