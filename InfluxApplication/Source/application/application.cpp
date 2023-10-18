#include "app_pch.h"

#include "application/application.h"
#include "influx_renderer.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#pragma comment(lib, "InfluxRenderer.lib")
#pragma comment(lib, "InfluxAsync.lib")

namespace influx::application
{
	void application::run(const run_args& args)
	{
		// initialize
		{
			m_instancehandle = platform::get_current_instance();

			if (!args.m_commandlet)
			{
				// create a window
				platform::create_window_args window_args{};
				window_args.m_width = (int)args.m_window_width;
				window_args.m_height = (int)args.m_window_height;
				window_args.m_name = args.m_name;
				m_windowhandle = platform::create_window(window_args);

				// create the renderer
				m_renderthread = std::thread(&application::run_renderthread, this);
			}
		}
		
		// run a separate gamethread
		// m_gamethread = std::thread(&application::run_gamethread, this);

		run_mainthread();

		if (m_renderthread.joinable()) m_renderthread.join();
		if (m_gamethread.joinable()) m_gamethread.join();
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
	}

	void application::run_mainthread()
	{
		while (!m_is_quit_requested)
		{

		}
	}

	void application::run_gamethread()
	{
		while (!m_is_quit_requested)
		{

			++m_gamethread_frame;
		}
	}

	void application::run_renderthread()
	{
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		renderer::initialize(args);

		renderer::present_args present_args{};
		while (!m_is_quit_requested)
		{
			renderer::command_list* record_list = renderer::record();
			renderer::submit(record_list);
			renderer::present_to_window(m_windowhandle, present_args);
			++m_renderthread_frame;
		}

		renderer::cleanup();
	}

#pragma region apifunctions
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
