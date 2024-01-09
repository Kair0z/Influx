#include "app_pch.h"
#include "application/application_backend.h"

#include "influx_async.h"

namespace influx::application
{
	void application::run(const run_args& args)
	{
		process_run_args(args);

		m_instancehandle = platform::get_current_instance();

		if (m_run_args.m_commandlet == false)
		{
			// create a platform window
			platform::create_window_args window_args{};
			window_args.m_width		= (int)args.m_window_width;
			window_args.m_height	= (int)args.m_window_height;
			window_args.m_name		= args.m_name;
			m_windowhandle = platform::create_window(window_args, true);
		}
		
		// initialize async job scheduler
		async::init_args async_init_args{};
		async_init_args.m_num_workers = 1u;
		async::initialize(async_init_args);

		while (true)
		{

		}
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
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

	void application::process_run_args(const run_args& args)
	{
		m_run_args = args;

		if (m_run_args.m_resources_dir.empty())
		{
			m_run_args.m_resources_dir = platform::get_current_directory() + "/Resources/";
		}
	}

	string application::get_resource_directory() const
	{
		return m_run_args.m_resources_dir;
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

	void quit()
	{
		application::get_instance().request_quit();
	}
#pragma endregion
}
