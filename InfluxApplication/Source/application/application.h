#pragma once

#include "influx_application.h"

#include "core/singleton/singleton.h"
#include "core/platform/platform.h"

#include <atomic>
#include <thread>

namespace influx::application
{
	class application final 
		: public singleton<application>
	{
	public:
		void run(const run_args& args);
		void request_quit();

	private:
		void run_mainthread();
		void run_renderthread();

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		std::atomic_bool m_is_quit_requested = false;

		std::thread m_mainthread;
		std::thread m_renderthread;
		uint64 m_gamethread_frame = 0u;
		uint64 m_renderthread_frame = 0u;
		
		run_args m_run_args{};
	};
}


