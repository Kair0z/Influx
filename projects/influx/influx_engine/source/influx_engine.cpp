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

namespace influx::engine
{
	void engine::run(base_module* mod)
	{
		m_module = mod;
		influx_assert(m_module != nullptr);

		initialize();
		m_t_start = time::get_now();

		if (game_module* as_game = dynamic_cast<game_module*>(m_module))
		{
			m_game = as_game;
			run_game();
		}

		if (editor_module* as_editor = dynamic_cast<editor_module*>(m_module))
		{
			m_editor = as_editor;
			run_editor();
		}

#if INFLUX_DEBUG
		influx::log_scopedata();
#endif

		m_is_quit = true;
		cleanup();
	}

	void engine::initialize()
	{
		m_t_init = time::get_now();

		// setup engine config
		m_config.m_file_influx_root = get_engine_directory(engine::e_directory::root);
		m_config.m_file_influx_resources = get_engine_directory(engine::e_directory::resources);
		m_config.m_file_influx_assets = get_engine_directory(engine::e_directory::assets);
		m_config.m_file_influx_staged = get_engine_directory(engine::e_directory::staged);

		// initialize job system:
		async::init_args async_args{};
		async_args.m_num_workers = 2u;
		async::initialize(async_args);

		// initialize input and run an input thread
		influx::input::init();
		m_inputthread = thread([this]()
		{
			run_input();
		});

		// make editor content
		m_contentman = new content_manager(this);
	}

	void engine::run_game()
	{
		game_config game_config{};
		m_game->on_config(m_app_config, game_config);

		initialize_renderer(game_config.m_gamename, m_app_config);

		frame_time frame_time{};
		game_module::ctx_update update_ctx{};
		while (!m_is_quit_requested)
		{
			frame_time.tick();

			poll_platform_events();

			// update
			update_ctx.m_frametime = frame_time;
			m_game->on_update(update_ctx);

			// render
			renderer::scene scene{};
			m_renderman->render(&scene);
		}
	}

	void engine::run_editor()
	{
		m_editorman = new editor_manager(m_editor);

		editor_config config{};
		m_editor->on_config(m_app_config, config);

		initialize_renderer("influx editor", m_app_config);

		frame_time frame_time{};
		while (!m_is_quit_requested)
		{
			frame_time.tick();
			m_fps = 1.0f / frame_time.m_delta_seconds;

			poll_platform_events();

			m_renderman->record_imgui_frame([this](ImGuiContext& ctx)
			{
				m_editorman->on_imgui(ctx);
			});

			renderer::scene scene{};
			m_renderman->render(&scene);
		}
	}

	void engine::run_input()
	{
		while (!m_is_quit_requested)
		{
			input::service();
		}
	}

	void engine::cleanup()
	{
		if (m_module)
		{
			delete m_module;
			m_module = nullptr;
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

		if (m_contentman)
		{
			delete m_contentman;
			m_contentman = nullptr;
		}

		if (m_inputthread.joinable())
			m_inputthread.join();
		
		async::shutdown();
	}

	void engine::initialize_renderer(const string& window_name, const app_config& config)
	{
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(config.m_window_dimensions)
			.set_name(window_name);

		m_window = platform::window::create(window_desc);
		if (m_window == nullptr)
		{
			logonce(e_log_category::warning, "influx_engine::engine: window::create() failed!");
		}

		m_window->set_event_callback([this](const platform::window_event& ev)
		{
			on_window_event(ev);
		});

		m_renderman = new render_manager(this);
	}

	void engine::poll_platform_events()
	{
		m_window->poll_events(m_is_quit_requested);
		m_is_quit_requested |= m_window->has_quit_request();
	}

	void engine::on_window_event(const platform::window_event& ev)
	{
		input::push_window_event(ev);
	}

	file engine::get_engine_directory(e_directory dir)
	{
		// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
		const string& root = platform::platform::get_current_directory() + "/../../../";
		switch (dir)
		{
		case e_directory::root:			return root;
		case e_directory::resources:	return root + "/resources/";
		case e_directory::assets:		return root + "/assets/";
		case e_directory::staged:		return root + "/staged/";
		case e_directory::binaries:		return root + "/bin/";
		case e_directory::intermediate: return root + "/int/";
		case e_directory::games:		return root + "/games/";
		}
		return {};
	}

	platform::window const* engine::get_window() const
	{
		return m_window;
	}

	content_manager const* engine::get_content() const
	{
		return m_contentman;
	}

	render_manager const* engine::get_renderer() const
	{
		return m_renderman;
	}

	float engine::get_fps() const
	{
		return m_fps;
	}
	
	bool engine::is_quit() const
	{
		return m_is_quit;
	}
}

namespace influx::engine::detail
{
	bool run_engine(base_module* mod)
	{
		get_engine()->run(mod);
		return true;
	}
}