#include "app_pch.h"

#include "application/application.h"
#include "influx_renderer.h"
#include "influx_async.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#include "Core/Math/Random.h"

#pragma comment(lib, "InfluxRenderer.lib")
#pragma comment(lib, "InfluxAsync.lib")

#include <iostream>

namespace influx::application
{
	#if INFLUX_APP_USES_WINDOWS
	inline static ::LRESULT windows_procedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			application::get_instance().request_quit();
			return 0;
		}

		default:
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	#endif

	void application::run(const run_args& args)
	{
		m_run_args = args;

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
				m_windowhandle = platform::create_window(window_args, true, windows_procedure);

				// create the renderer
				m_renderthread = std::thread(&application::run_renderthread, this);

				// run main thread
				// m_mainthread = std::thread(&application::run_mainthread, this);
				run_mainthread();
			}
		}
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;

		// cleanup
		if (m_renderthread.joinable()) m_renderthread.join();
		if (m_mainthread.joinable()) m_mainthread.join();
	}

	void application::run_mainthread()
	{
		vector<platform::e_windowevent> out_events{};
		while (!m_is_quit_requested)
		{
			if (!platform::poll_window_events(out_events, m_windowhandle))
			{
				request_quit();
			}
			else
			{
				for (platform::e_windowevent e : out_events)
				{
					switch (e)
					{
					default:
						break;
					}
				}
			}
		}
	}

	void application::run_renderthread()
	{
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		// ...
		renderer::initialize(args);

		renderer::render_args render_args{};
		// ...

		renderer::present_args present_args{};
		present_args.m_vsync = m_run_args.m_vsync;
		// ...

		while (!m_is_quit_requested)
		{
			{
				// renderer::start_render

				// renderer::application
				renderer::render_to_window(render_args, m_windowhandle, present_args);
			}
			
			if (m_renderthread_frame % 360u == 0u && m_renderthread_frame != 0u)
			{
				renderer::frame_stats stats = renderer::frame_stats::average(renderer::get_frame_stats(640u));
				std::cout << "FPS: " << 1.0f / (stats.m_ms_frame * 0.001f) << "\n";
			}
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
