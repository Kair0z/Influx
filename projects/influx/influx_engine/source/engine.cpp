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
#include "influx_engine/world/world.h"

namespace influx::engine
{
	void engine::run(run_type type)
	{
		initialize();
		m_t_start = time::get_now();

		string render_name = "";
		if (type == run_type::editor)
		{
			m_editorman = new editor_manager(nullptr);

			render_name = "influx_editor";
		}
		else
		{
			render_name = "influx_game";
		}

		// init render
		app_config app_config{};
		app_config.m_window_dimensions = { 640u, 480u };
		initialize_renderer(render_name, app_config);

		// init world
		m_world = new world();

		// init content
		m_contentman->load_engine_assets(this);

		// run
		while (!m_is_quit_requested)
		{
			m_time.tick();
			m_fps = 1.0f / m_time.get_delta_seconds();

			poll_platform_events();
			if (m_is_quit_requested) break;

			// stream available assets from content into the renderer
			m_renderman->load_render_assets(m_contentman);

			// record imgui
			if (type == run_type::editor)
			{
				m_renderman->record_imgui_frame([this](ImGuiContext& ctx)
				{
					m_editorman->update_imgui(ctx);
				});
			}
			
			// build a render-scene
			renderer::scene scene{};
			scene.m_seconds = m_time.get_time_seconds();
			scene.m_delta_seconds = m_time.get_delta_seconds();
			renderer::scene2D scene2D{};
			m_world->build_renderscene(scene, scene2D);
			m_renderman->render(scene);
		}

		delete m_world;

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
		m_config.m_file_influx_root = get_engine_directory(engine_directory::root);
		m_config.m_file_influx_assets = get_engine_directory(engine_directory::assets);
		m_config.m_file_influx_staged = get_engine_directory(engine_directory::staged);

		// initialize job system:
		async::init_args async_args{};
		async_args.m_num_workers = 1u;
		async::initialize(async_args);

		// initialize input and run an input thread
		influx::input::init();
		m_inputthread = thread([this]()
		{
			run_input();
		});

		m_contentman = new content_manager(this);
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
		async::shutdown();

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

	platform::window const* engine::get_window() const
	{
		return m_window;
	}

	content_manager* engine::get_content() const
	{
		return m_contentman;
	}

	render_manager const* engine::get_renderer() const
	{
		return m_renderman;
	}

	const frame_time& engine::get_time() const
	{
		return m_time;
	}

	world* engine::get_world() const
	{
		return m_world;
	}

	float engine::get_fps() const
	{
		return m_fps;
	}
	
	bool engine::is_quit() const
	{
		return m_is_quit;
	}

	world* get_world()
	{
		return get_engine()->get_world();
	}
}

#include "influx_engine.h"
namespace influx::engine
{
	void run_editor()
	{
		get_engine()->run(engine::run_type::editor);
	}

	void run_game()
	{
		get_engine()->run(engine::run_type::game);
	}
}