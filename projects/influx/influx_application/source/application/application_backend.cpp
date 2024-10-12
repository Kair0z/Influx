#include "app_pch.h"
#include "application/application_backend.h"

// influx::application
#include "scene/scene.h"
#include "renderer/renderer.h"
#include "content/content_manager.h"
#include "editor/editor.h"

// influx::platform - win32
#include "core/platform/win32/win32_platform.h"
#include "core/platform/win32/win32_window.h"

// influx::async
#include "influx_async.h"

// influx::input
#include "influx_input.h"

// influx::import
#include "influx_import.h"

namespace influx::application
{
	void application::run(const run_args& args)
	{
		process_run_args(args);

		initialize(args);

		// some stack variables
		time::point initial_tick = time::get_now();
		time::point last_tick = initial_tick;
		frame_time frame_time{};

		logn("start ticking ...");
		while (!m_is_quit_requested)
		{
			frame_time.tick();

			// returns false if quit event was requested
			if (!platform::poll_window_events(m_windowhandle))
			{
				request_quit();
				continue;
			}

			// update
			mp_scene->update(frame_time);
			mp_editor->update();

			// render
			mp_renderer->render(mp_scene->get_render_scene());
		}
	}

	void application::initialize(const run_args& args)
	{
		// create the application window
		m_instancehandle = platform::get_current_instance();
		platform::create_window_args window_args{};
		window_args.m_width = args.m_window_width;
		window_args.m_height = args.m_window_height;
		window_args.m_name = args.m_name;
		// window_args.m_proc_callback = [](const platform::window_event& ev) { input::push_window_event(ev); };
		m_windowhandle = platform::create_window(window_args);

		// initialize job system:
		async::init_args async_args{};
		async_args.m_num_workers = 4u;
		async::initialize(async_args);

		// initialize input
		influx::input::init();
		m_input_thread = thread([this]()
		{
			while (!m_is_quit_requested)
			{
				input::service();
			}
		});

		mp_content_manager = new content_manager(get_resource_directory());

		mp_editor = new editor();
		mp_editor->subscribe([this]() { on_imgui(); });

		mp_scene = new scene();

		mp_renderer = new renderer(m_windowhandle);
		mp_renderer->load_render_assets(mp_content_manager);

		if (m_user_init_clb)
			m_user_init_clb();
	}

	void application::cleanup()
	{
		if (m_user_shutdown_clb)
			m_user_shutdown_clb();

		influx::input::cleanup();
		
		// join the inputthread
		m_input_thread.join();
	}

	void application::on_imgui()
	{
		if (m_user_imgui_clb)
			m_user_imgui_clb();
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
	}

	string application::get_resource_directory()
	{
		return get_instance().m_resource_dir;
	}

	string application::get_assets_directory()
	{
		return get_instance().m_asset_dir;
	}

	string application::get_intermediate_directory()
	{
		return get_instance().m_int_dir;
	}

	bool application::is_quit_requested()
	{
		return get_instance().m_is_quit_requested;
	}

	bool application::is_vsync()
	{
		return get_instance().m_run_args.m_vsync || k_force_vsync;
	}

	bool application::is_editor_enabled()
	{
		return get_instance().m_run_args.m_enable_editor && !is_commandlet();
	}

	bool application::is_game_enabled()
	{
		return get_instance().m_run_args.m_enable_game && !is_commandlet();
	}

	bool application::is_scene_render_enabled()
	{
		return true && k_allow_scenerender;
	}

	bool application::is_commandlet()
	{
		return get_instance().m_run_args.m_commandlet;
	}

	void application::set_on_initialize(const init_callback& clb)
	{
		m_user_init_clb = clb;
	}

	void application::set_on_imgui(const imgui_callback& clb)
	{
		m_user_imgui_clb = clb;
	}

	void application::set_on_shutdown(const shutdown_callback& clb)
	{
		m_user_shutdown_clb = clb;
	}

	editor* application::get_editor()
	{
		return get_instance().mp_editor;
	}

	void application::process_run_args(const run_args& args)
	{
		m_run_args = args;
		m_staged = args.m_staged;

		if (m_staged)
		{
			m_resource_dir = (m_run_args.m_resources_dir.empty()) ?
				platform::get_current_directory() + "/resources/" : m_run_args.m_resources_dir;

			m_asset_dir = (m_run_args.m_assets_dir.empty()) ?
				platform::get_current_directory() + "/assets/" : m_run_args.m_assets_dir;
		}
		else
		{
			// non-staged builds are ran in Influx/bin/[config]/influx_game/ folder
			const string& root_influx = platform::get_current_directory() + "/../../../";
			m_resource_dir = root_influx + "/resources/";
			m_asset_dir = root_influx + "/assets/";
			m_int_dir = root_influx + "/int/";
		}
	}

	run_args application::get_run_arguments() const
	{
		return m_run_args;
	}

	platform::window_handle application::get_window_handle() const
	{
		return m_windowhandle;
	}

	platform::instance_handle application::get_instance_handle() const
	{
		return m_instancehandle;
	}

#pragma region frontend api
	void run(const run_args& args)
	{
		application::get_instance().run(args);
	}

	void on_initialize(const init_callback& clb)
	{
		application::get_instance().set_on_initialize(clb);
	}

	void on_imgui(const imgui_callback& clb)
	{
		application::get_instance().set_on_imgui(clb);
	}

	void on_shutdown(const shutdown_callback& clb)
	{
		application::get_instance().set_on_shutdown(clb);
	}

	void quit()
	{
		application::get_instance().request_quit();
	}
#pragma endregion
}
