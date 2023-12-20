#include "app_pch.h"

#include "application/application_backend.h"
#include "application/threads/rendersync.h"

#include "application/layers/layer_stack.h"
#include "application/layers/layers/layers.h"

#include "influx_async.h"
#include "ImGui/imgui_impl_win32.h"

#include <iostream>

#if INFLUX_PLATFORM_WINDOWS
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace influx::application
{
	base* create_module()
	{
		return nullptr;
	}

	#if INFLUX_PLATFORM_WINDOWS
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

	void application::run(const run_args& args, base* sub_module)
	{
		m_run_args = args;

		if (m_run_args.m_resources_dir.empty())
		{
			m_run_args.m_resources_dir = platform::get_current_directory() + "/Resources/";
		}
		m_instancehandle = platform::get_current_instance();

		// create a window
		if (m_run_args.m_commandlet == false)
		{
			platform::create_window_args window_args{};
			window_args.m_width = (int)args.m_window_width;
			window_args.m_height = (int)args.m_window_height;
			window_args.m_name = args.m_name;
			m_windowhandle = platform::create_window(window_args, true, windows_procedure);
		}
		
		// initialize async jobs
		async::init_args async_init_args{};
		async_init_args.m_num_workers = k_max_num_job_threads;
		async::initialize(async_init_args);

		// create base app module
		mp_base_application = sub_module;

		// create layerstack
		mp_layerstack = new layer_stack();
		mp_layerstack->push<layer_main>		(layer_base_args{"layer_main"});	// |
		mp_layerstack->push<layer_module>	(layer_base_args{"layer_module"});	// |
		mp_layerstack->push<layer_editor>	(layer_base_args{"layer_editor"});	// v

		while (!m_is_quit_requested)
		{
			mp_layerstack->process_events();
			mp_layerstack->tick();
		}

		// shutdown
		influx_delete(mp_layerstack);
		async::shutdown();
	}

	void application::main_init()
	{
		m_mainthread_frame = 0u;

		

		ImGui_ImplWin32_Init(m_windowhandle);
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
		return k_render_scene;
	}

	bool application::is_commandlet()
	{
		return get_instance().m_run_args.m_commandlet;
	}

	rendersync& application::get_render_sync()
	{
		return *get_instance().mp_rendersynce;
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
	void run(const run_args& args, base* sub_module)
	{
		application::get_instance().run(args, sub_module);
	}

	void quit()
	{
		application::get_instance().request_quit();
	}
#pragma endregion
}
