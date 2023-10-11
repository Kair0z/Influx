#include "app_pch.h"

#include "application/application.h"
#include "application/influx_application.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

namespace influx::application
{
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

	void application::run(const run_args& args)
	{
		// initialize
		{
			m_instancehandle = platform::get_current_instance();

			if (!args.m_commandlet)
			{
				platform::create_window_args window_args{};
				window_args.m_width = (int)args.m_window_width;
				window_args.m_height = (int)args.m_window_height;
				window_args.m_name = args.m_name;

				m_windowhandle = platform::create_window(window_args);
			}
		}

		m_renderthread = std::thread(&application::run_renderthread, this);
		m_gamethread = std::thread(&application::run_gamethread, this);

		while (!m_is_quit_requested)
		{

		}

		m_renderthread.join();
		m_gamethread.join();
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
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
		while (!m_is_quit_requested)
		{

			++m_renderthread_frame;
		}
	}
}
