#include "app_pch.h"

#include "application/application.h"
#include "influx_renderer.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#pragma comment(lib, "InfluxRenderer.lib")

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
		// initialize renderer
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		renderer::initialize(args);

		renderer::command_list* game_render_command = nullptr;
		renderer::command_list* editor_render_command = nullptr;

		// now this! is threaded rendering!
		std::thread render_editor_command_thread{};
		std::thread render_game_command_thread{};
		render_editor_command_thread = std::thread([this, &editor_render_command]()
		{
			while (!m_is_quit_requested)
			{
				if (editor_render_command != nullptr) continue;

				renderer::command_list* list = renderer::record();

				editor_render_command = list;
			}
		});
		render_game_command_thread = std::thread([this, &game_render_command]()
		{
			while (!m_is_quit_requested)
			{
				if (game_render_command != nullptr) continue;

				renderer::command_list* list = renderer::record();

				game_render_command = list;
			}
		});

		// main render thread
		// wait for editor & game commandlist to finish
		// then submit
		renderer::present_args present_args{};
		while (!m_is_quit_requested)
		{
			if (game_render_command != nullptr && editor_render_command != nullptr)
			{
				renderer::submit({ game_render_command, editor_render_command });
				renderer::present_to_window(m_windowhandle, present_args);
				++m_renderthread_frame;

				editor_render_command = nullptr;
				game_render_command = nullptr;
			}
		}

		if (render_game_command_thread.joinable()) render_game_command_thread.join();
		if (render_editor_command_thread.joinable()) render_editor_command_thread.join();
		renderer::cleanup();
	}
}
