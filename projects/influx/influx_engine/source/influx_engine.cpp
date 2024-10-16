#include "engine_pch.h"

// influx::platform
#include "influx_platform/platform.h"

// influx::core
#include "core/time.h"
#include "core/log.h"

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
	// sets up files & directory paths
	void setup_engineconfig(game_module::config& config)
	{
		engine* engine = get_engine();
		config.m_file_influx_root = engine->get_engine_directory(engine::e_directory::root);
		config.m_file_influx_resources = engine->get_engine_directory(engine::e_directory::resources);
		config.m_file_influx_assets = engine->get_engine_directory(engine::e_directory::assets);
		config.m_file_influx_staged = engine->get_engine_directory(engine::e_directory::staged);
	}

	void engine::run(run_type type)
	{
		switch (type)
		{
		case run_type::editor:
			run_editor();
			break;

		case run_type::game:
			run_game();
			break;
		}
	}

	void engine::run_game()
	{
		m_gamemodule = detail::create_game();

		// build game config:
		game_module::config game_config{};
		setup_engineconfig(game_config);
		m_gamemodule->on_config(game_config);

		// parse game config:
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(game_config.m_window_dimensions)
			.set_name(game_config.m_gamename);

		// make a window
		m_window = platform::window::create(window_desc);

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

		// make game content
		m_contentman = new content_manager(this);

		// make renderer
		m_renderman = new render_manager(this);
		//m_renderman->load_render_assets(m_contentman);

		// some stack variables
		time::point initial_tick = time::get_now();
		time::point last_tick = initial_tick;
		frame_time frame_time{};
		game_module::ctx_update update_ctx{};
		while (!m_is_quit_requested)
		{
			frame_time.tick();

			m_window->poll_events(m_is_quit_requested);
			m_is_quit_requested |= m_window->has_quit_request();

			update_ctx.m_frametime = frame_time;
			m_gamemodule->on_update(update_ctx);

			// make renderscene
			renderer::scene scene{};

			m_renderman->render(scene);
		}

		m_is_quit = true;

		// CLEANUP
		m_gamemodule->on_cleanup();
		delete m_window;
		delete m_renderman;
		delete m_contentman;
		delete m_gamemodule;
	}

	void engine::run_editor()
	{

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
		get_engine()->run(engine::run_type::game);
	}
}