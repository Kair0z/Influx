#include "engine_pch.h"

// influx::platform
#include "influx_platform/platform.h"

// influx::async
#include "influx_async.h"

// influx::input
#include "influx_input.h"

// influx::renderer
#include "influx_renderer/scene.h"

// influx::engine
#include "content/content_manager.h"
#include "rendering/render_manager.h"

namespace influx::engine
{
	void engine::run()
	{
		initialize();

		m_gamemodule = detail::create_game();
		m_editormodule = detail::create_editor();

		m_t_start = time::get_now();

		if (m_gamemodule && m_editormodule == nullptr)
		{
			run_game();
			delete m_gamemodule;
			m_gamemodule = nullptr;
		}
		if (m_editormodule && m_gamemodule == nullptr)
		{
			run_editor();
			delete m_editormodule;
			m_editormodule = nullptr;
		}

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
		async_args.m_num_workers = 4u;
		async::initialize(async_args);

		// initialize input and run an input thread
		influx::input::init();
		m_inputthread = thread([this]()
		{
			while (!m_is_quit_requested)
			{
				input::service();
			}
		});

		// make editor content
		m_contentman = new content_manager(this);
	}

	void engine::cleanup()
	{
		delete m_contentman;
		m_contentman = nullptr;

		if (m_inputthread.joinable())
			m_inputthread.join();
		
		async::shutdown();

		influx::log_scopedata();
	}

	void engine::run_game()
	{
		m_gamemodule = detail::create_game();

		game_config game_config{};
		m_gamemodule->on_config(m_app_config, game_config);

		initialize_renderer(m_app_config);

		frame_time frame_time{};
		game_module::ctx_update update_ctx{};
		while (!m_is_quit_requested)
		{
			frame_time.tick();

			m_window->poll_events(m_is_quit_requested);
			m_is_quit_requested |= m_window->has_quit_request();

			update_ctx.m_frametime = frame_time;
			m_gamemodule->on_update(update_ctx);

			renderer::scene scene{};
			m_renderman->render(&scene);
		}

		delete m_renderman;
		m_renderman = nullptr;

		delete m_window;
		m_window = nullptr;

		m_gamemodule->on_cleanup();
		delete m_gamemodule;
	}

	void engine::run_editor()
	{
		// build editor config:
		editor_config config{};
		m_editormodule->on_config(m_app_config, config);

		initialize_renderer(m_app_config);

		frame_time frame_time{};
		while (!m_is_quit_requested)
		{
			// tick time
			frame_time.tick();

			// platform event poll
			m_window->poll_events(m_is_quit_requested);
			m_is_quit_requested |= m_window->has_quit_request();

			m_renderman->prerender();

			m_renderman->record_imgui_frame([this]()
			{
				m_editormodule->on_imgui();
			});

			// render
			renderer::scene scene{};
			m_renderman->render(&scene);
		}

		m_editormodule->on_cleanup();
		delete m_window;
	}

	void engine::initialize_renderer(const app_config& config)
	{
		// make a window
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(config.m_window_dimensions)
			.set_name("influx editor");

		m_window = platform::window::create(window_desc);
		if (m_window == nullptr)
		{
			logonce(e_log_category::warning, "influx_engine::engine: window::create() failed!");
		}

		// make renderer
		m_renderman = new render_manager(this);
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
	
	bool engine::is_quit() const
	{
		return m_is_quit;
	}
}

namespace influx::engine::detail
{
	void run_engine()
	{
		get_engine()->run();
	}
}