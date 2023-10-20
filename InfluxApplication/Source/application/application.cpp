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
		async::init_args args{};
		args.m_num_workers = 8u;
		async::initialize(args);

		uint64 static_num_bytes = async::get_static_num_bytes();

		random::seed_random();
		vector<int> randoms = random::get_randoms<int, 640u>(0, 150);

		const uint32 num_tasks = randoms.size() / 1u;
		vector<async::task_handle> tasks{};

		auto base_lambda = [&randoms, num_tasks](uint32 base_idx)
		{
			uint32 range = math::ceil<uint32>((float)randoms.size() / num_tasks);

			for (uint32 i = 0u; i < range; ++i)
			{
				randoms[(base_idx * range) + i] += 3u;
			}
		};
		
		while (!m_is_quit_requested)
		{
			for (uint32 i = 0u; i < num_tasks; ++i)
			{
				async::task_args args{[i, &base_lambda]() 
				{
					base_lambda(i);
				}};

				tasks.push_back(async::create_task(args));
			}

			for (const async::task_handle& handle : tasks)
			{
				async::dispatch(handle);
			}

			for (const async::task_handle& handle : tasks)
			{
				handle.wait();
			}

			int total = 0u;
			for (const int& i : randoms)
			{
				total += i;
			}

			std::cout << "Total: " << total << "\n";
			tasks.clear();
		}

		async::shutdown();
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
