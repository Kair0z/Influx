#include "app_pch.h"

#include "application/application.h"
#include "application/threads/gamethread.h"
#include "application/threads/renderthread.h"

#include "Core/Math/Random.h"
#include "core/geometry/quad.h"
#include "core/geometry/geometry.h"
#include "Core/Time.h"
#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#include "influx_async.h"
#pragma comment(lib, "InfluxAsync.lib")

#pragma region imgui
#include "foreign/ImGui/imgui.h"
#if INFLUX_APP_USES_WINDOWS
	#include "foreign/ImGui/imgui_impl_win32.h"
#endif
#pragma endregion

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
		if (m_run_args.m_resources_dir.empty())
		{
			m_run_args.m_resources_dir = platform::get_current_directory() + "/Resources/";
		}

		m_instancehandle = platform::get_current_instance();

		// initialize async jobs module
		async::init_args async_init_args{};
		async_init_args.m_num_workers = k_max_num_job_threads;
		async::initialize(async_init_args);

		// create a window
		if (m_run_args.m_commandlet == false)
		{
			platform::create_window_args window_args{};
			window_args.m_width = (int)args.m_window_width;
			window_args.m_height = (int)args.m_window_height;
			window_args.m_name = args.m_name;
			m_windowhandle = platform::create_window(window_args, true, windows_procedure);
		}
		
		// create dedicated threads
		mp_gamethread = new gamethread();
		mp_renderthread = new renderthread();
		{
			m_dedicated_threads.clear();
			m_dedicated_threads.push_back(mp_gamethread);
			m_dedicated_threads.push_back(mp_renderthread);
		}
		
		if (args.m_single_threaded)
		{
			main_init();
			mp_gamethread->initialize();
			mp_renderthread->initialize();
			while (!m_is_quit_requested)
			{
				main_tick();
				mp_gamethread->tick();
				mp_renderthread->tick();
			}
			main_cleanup();
			mp_gamethread->cleanup();
			mp_renderthread->cleanup();
		}
		else
		{
			main_init();
			for (dedicated_thread*& thread : m_dedicated_threads)
			{
				thread->spin();
			}
			while (!m_is_quit_requested)
			{
				main_tick();
			}
			// joins() & so stalls until all threads are finished...
			for (dedicated_thread*& thread : m_dedicated_threads)
			{
				delete thread;
				thread = nullptr;
			}
			main_cleanup();
		}

		async::shutdown();
	}

	void application::main_init()
	{
		m_mainthread_frame = 0u;

		if (m_run_args.m_enable_editor)
		{
			ImGui_ImplWin32_Init(application::get_instance().get_window_handle());
		}
	}

	void application::main_tick()
	{
		// window events
		vector<platform::e_windowevent> out_events{};
		if (!platform::poll_window_events(out_events, m_windowhandle))
		{
			request_quit();
			return;
		}

		// handle events
		for (platform::e_windowevent e : out_events)
		{
		}

		// editor
		if (m_run_args.m_enable_editor)
		{
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::ShowDemoWindow();
			}
			ImGui::Render(); // endframe + submits draw data

			ImGui::GetDrawData();
		}

		// log stats
		const uint64 game_frame = mp_gamethread->get_frame();
		if (game_frame % k_stats_log_frame_intv == 0u && game_frame != 0u)
		{
			mainthread_log();
		}

		++m_mainthread_frame;
	}

	void application::main_cleanup()
	{
		if (m_run_args.m_enable_editor)
		{
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
	}

	void application::mainthread_log()
	{
		system("cls");
		dedicated_thread::per_frame_stats game_stats = mp_gamethread->get_average_stats(k_num_stats_to_average); // we can miss a couple of frames but who cares?
		dedicated_thread::per_frame_stats render_stats = mp_renderthread->get_average_stats(k_num_stats_to_average);

		auto set_console_color = [](const float ms_value, const float pc_sync)
		{
			platform::set_console_colour_attribute(platform::e_console_colour::white);
			const float pc_self = 1.0f - pc_sync;
			const float ms_self = ms_value * pc_self;
			
			if (ms_self >= 16.66f)
			{
				platform::set_console_colour_attribute(platform::e_console_colour::yellow);
			}
			if (ms_self >= 33.33f)
			{
				platform::set_console_colour_attribute(platform::e_console_colour::red);
			}
		};

		const float game_fps = 1.0f / (game_stats.m_ms_total * 0.001f);
		const float render_fps = 1.0f / (render_stats.m_ms_total * 0.001f);
		const float game_ms_sync = game_stats.m_pc_sync * game_stats.m_ms_total;
		const float render_ms_sync = render_stats.m_pc_sync * render_stats.m_ms_total;

		set_console_color(game_stats.m_ms_total, game_stats.m_pc_sync);
		std::cout << "[Game]  \tFPS: " << game_fps << "\t| ms: " << game_stats.m_ms_total << "\t\t| " << "Sync: " << 100.0f * game_stats.m_pc_sync 
			<< "% (" << game_ms_sync << " ms)\n";
		set_console_color(render_stats.m_ms_total, render_stats.m_pc_sync);
		std::cout << "[Render]\tFPS: " << render_fps << "\t| ms: " << render_stats.m_ms_total << "\t\t| " << "Sync: " << 100.0f * render_stats.m_pc_sync 
			<< "% (" << render_ms_sync << " ms)\n";

		platform::set_console_colour_attribute(platform::e_console_colour::white);
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
	}

	rendersync& application::get_render_sync()
	{
		return get_instance().m_render_sync;
	}

	bool application::is_quit_requested()
	{
		return get_instance().m_is_quit_requested;
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
