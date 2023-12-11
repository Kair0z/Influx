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

#include "foreign/ImGui/imgui_impl_win32.h"

#include <iostream>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace influx::application
{
	#if INFLUX_APP_USES_WINDOWS
	inline static ::LRESULT windows_procedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		if (application::is_editor_enabled())
		{
			// nasty imgui dependency here...
			if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			{
				// return?...
			}
		}

		switch (uMsg)
		{
		case WM_DESTROY:
		{
			application::get_instance().request_quit();
			return 0;
		}

		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_NCMOUSELEAVE:
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_INPUTLANGCHANGE:
		case WM_CHAR:
		case WM_SETCURSOR:
		default:
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	#endif

	void application::run(const run_args& args, bool blocking)
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
		
		if (is_single_threaded())
		{
			main_init();
			mp_gamethread->call_initialize();
			mp_renderthread->call_initialize();
			while (!m_is_quit_requested)
			{
				main_tick();
				mp_gamethread->call_tick();
				mp_renderthread->call_tick();
			}
			main_cleanup();
			mp_gamethread->call_cleanup();
			mp_renderthread->call_cleanup();
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

	}

	void application::mainthread_log()
	{
		system("cls");
		per_frame_stats game_stats = mp_gamethread->calc_average_stats();
		per_frame_stats render_stats = mp_renderthread->calc_average_stats();

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

		const bool log_sync_stats = !is_single_threaded(); // no need for sync stats if single threaded

		set_console_color(game_stats.m_ms_total, game_stats.m_pc_sync);
		std::cout << "[Game]  \tFPS: " << game_fps << "\t| ms: " << game_stats.m_ms_total;
		if (log_sync_stats) std::cout << "\t\t| " << "Sync: " << 100.0f * game_stats.m_pc_sync << "% (" << game_ms_sync << " ms)";
		std::cout << "\n";

		set_console_color(render_stats.m_ms_total, render_stats.m_pc_sync);
		std::cout << "[Render]\tFPS: " << render_fps << "\t| ms: " << render_stats.m_ms_total;
		if (log_sync_stats) std::cout << "\t\t| " << "Sync: " << 100.0f * render_stats.m_pc_sync << "% (" << render_ms_sync << " ms)\n";
		std::cout << "\n";

		std::cout << "\n";
		if (is_vsync()) std::cout << "[vsync]";
		if (is_single_threaded()) std::cout << "[singlethreaded]"; else std::cout << "[threaded]";
		if (k_jobify) std::cout << "[jobified:" << to_string(k_max_num_job_threads) << "]";
		std::cout << "\n";

		platform::set_console_colour_attribute(platform::e_console_colour::white);
	}

	const dedicated_thread* application::find_thread(e_dedicated_thread thread_type) const
	{
		auto found = std::find_if(m_dedicated_threads.cbegin(), m_dedicated_threads.cend(),
		[thread_type](dedicated_thread* t)
		{
			return t->get_thread_type() == thread_type;
		});

		if (found != m_dedicated_threads.cend())
		{
			return *found;
		}

		return nullptr;
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

	bool application::is_single_threaded()
	{
		return get_instance().m_run_args.m_single_threaded || k_force_single_threaded;
	}

	bool application::is_vsync()
	{
		return get_instance().m_run_args.m_vsync || k_force_vsync;
	}

	bool application::is_editor_enabled()
	{
		return get_instance().m_run_args.m_enable_editor && !is_commandlet();
	}

	bool application::is_commandlet()
	{
		return get_instance().m_run_args.m_commandlet;
	}

	per_frame_stats application::get_average_frame_stats(e_dedicated_thread thread)
	{
		if (!is_single_threaded())
		{
			auto found_thread = get_instance().find_thread(thread);
			if (found_thread != nullptr)
			{
				return found_thread->get_average_stats();
			}
		}

		return {};
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
	void run(const run_args& args, bool blocking)
	{
		application::get_instance().run(args, blocking);
	}

	void quit()
	{
		application::get_instance().request_quit();
	}
#pragma endregion
}
