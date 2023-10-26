#include "app_pch.h"

#include "application/application.h"
#include "influx_renderer.h"
#include "influx_async.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#include "Core/Math/Random.h"
#include "Core/Time.h"

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
			// create entities
			constexpr uint64 k_num_entities = 4096u * 4096u;
			m_entities.reserve(k_num_entities);
			for (uint64 i = 0u; i < k_num_entities; ++i)
			{
				m_entities.push_back(i);
			}

			m_instancehandle = platform::get_current_instance();
			
			if (!args.m_commandlet)
			{
				// create a window
				platform::create_window_args window_args{};
				window_args.m_width = (int)args.m_window_width;
				window_args.m_height = (int)args.m_window_height;
				window_args.m_name = args.m_name;
				m_windowhandle = platform::create_window(window_args, true, windows_procedure);

				m_renderthread = std::thread(&application::run_renderthread, this);
				m_gamethread = std::thread(&application::run_gamethread, this);

				// run main thread
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
			// window events
			if (!platform::poll_window_events(out_events, m_windowhandle))
			{
				request_quit();
				return;
			}
			
			// handle events
			for (platform::e_windowevent e : out_events)
			{
				switch (e)
				{
				default:
					break;
				}
			}

			// log
			if (m_renderthread_frame % 360u == 0u && m_renderthread_frame != 0u)
			{
				renderer::frame_stats stats = renderer::frame_stats::average(renderer::get_frame_stats(640u));
				std::cout << "FPS: " << 1.0f / (stats.m_ms_frame * 0.001f) << "\n";
			}
		}
	}

	void application::run_gamethread()
	{
		while (!m_is_quit_requested)
		{
			const uint64 frame_to_reach = math::minimum<uint64>(static_cast<uint64>(m_gamethread_frame - m_run_args.m_max_thread_frame_difference), 0u);
			wait_for_renderthread_reaching(frame_to_reach);

			for (entity& entity : m_entities)
			{
				// update
				entity.m_id++;
			}

			++m_gamethread_frame;
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
			// also don't 
			{
				// make sure this frame's been simulated
				wait_for_gamethread_reaching(m_renderthread_frame + 1u);

				// renderer::start_render

				// renderer::application
				// renderer::render_to_window(render_args, m_windowhandle, present_args);
			}

			++m_renderthread_frame;
		}

		renderer::cleanup();
	}

	void application::wait_for_renderthread_reaching(const uint64 frame_to_reach, const wait_args& args)
	{
		time::point before_wait = time::get_now();
		while (m_renderthread_frame < frame_to_reach)
		{
			// wait...
		}

		if (args.mp_out_seconds_waited != nullptr)
		{
			(*args.mp_out_seconds_waited) = time::get_ms_between<float>(time::get_now(), before_wait) * 0.001f;
		}
	}

	void application::wait_for_gamethread_reaching(const uint64 frame_to_reach, const wait_args& args)
	{
		time::point before_wait = time::get_now();
		while (m_gamethread_frame < frame_to_reach)
		{
			// wait...
		}

		if (args.mp_out_seconds_waited != nullptr)
		{
			(*args.mp_out_seconds_waited) = time::get_ms_between<float>(time::get_now(), before_wait) * 0.001f;
		}
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
