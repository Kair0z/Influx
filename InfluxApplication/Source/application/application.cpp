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
		uint64 mainthread_frame = 0u;
		vector<platform::e_windowevent> out_events{};
		while (!m_is_quit_requested)
		{
			// window events
			if (!platform::poll_window_events(out_events, m_windowhandle))
			{
				request_quit();
				break;
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
			if (mainthread_frame % (512 * 512) == 0u && mainthread_frame != 0u)
			{
				system("cls");
				frame_stats game_stats = m_gamethread_state.m_stats.get_average_value(64u);
				frame_stats render_stats = m_renderthread_state.m_stats.get_average_value(64u);

				std::cout << "[Game]  \tFPS: " << 1.0f / (game_stats.m_ms_total * 0.001f)	<< "\t| ms: " << game_stats.m_ms_total	<< "\t| " << "Sync: "	<< 100.0f * game_stats.m_pc_sync << "%\n";
				std::cout << "[Render]\tFPS: " << 1.0f / (render_stats.m_ms_total * 0.001f) << "\t| ms: " << render_stats.m_ms_total << "\t| " << "Sync: "	<< 100.0f * render_stats.m_pc_sync << "%\n";
			}

			++mainthread_frame;
		}
	}

	void application::run_gamethread()
	{
		frame_stats this_frame_stat{};
		float seconds_synced = 0.0f;
		time::point frame_start = time::get_now();

		while (!m_is_quit_requested)
		{
			frame_start = time::get_now();

			const uint64 frame_to_reach = math::minimum<uint64>(static_cast<uint64>(m_gamethread_frame - m_run_args.m_max_thread_frame_difference), 0u);
			wait_for_renderthread_reaching(frame_to_reach, wait_args{ &seconds_synced });

			for (entity& entity : m_entities)
			{
				// update
				entity.m_id++;
			}

			this_frame_stat.m_ms_total = math::maximum(math::k_epsilon, time::get_ms_between<float>(time::get_now(), frame_start));
			this_frame_stat.m_pc_sync = math::is_zero(this_frame_stat.m_ms_total) ? 0.0f : (seconds_synced * 1000.0f) / this_frame_stat.m_ms_total;
			m_gamethread_state.m_stats.push(this_frame_stat);
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

		frame_stats this_frame_stats{};
		float seconds_synced = 0.0f;
		time::point frame_start = time::get_now();
		while (!m_is_quit_requested)
		{
			frame_start = time::get_now();

			// make sure this frame's been simulated
			wait_for_gamethread_reaching(m_renderthread_frame + 1u, wait_args{ &seconds_synced });

			// do something to the entities
			for (entity& entity : m_entities)
			{
				// update
				entity.m_id++;
			}

			// render them
			renderer::render_to_window(nullptr, render_args, m_windowhandle, present_args);

			this_frame_stats.m_ms_total = math::maximum(math::k_epsilon, time::get_ms_between<float>(time::get_now(), frame_start));
			this_frame_stats.m_pc_sync = math::is_zero(this_frame_stats.m_ms_total) ? 0.0f : (seconds_synced * 1000.0f) / this_frame_stats.m_ms_total;
			m_renderthread_state.m_stats.push(this_frame_stats);
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
