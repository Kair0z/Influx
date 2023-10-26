#pragma once

#include "influx_application.h"

#include "core/singleton/singleton.h"
#include "core/platform/platform.h"
#include "Core/Container/Vector.h"

#include <atomic>
#include <thread>

namespace influx::application
{
	class application final 
		: public singleton<application>
	{
		struct wait_args final
		{
			float* mp_out_seconds_waited = nullptr;
		};

		struct entity final
		{
			entity() = default;
			entity(uint64 id) : m_id{ id } {}
			uint64 m_id = 0u;
		};

		struct thread_state
		{

		};

	public:
		void run(const run_args& args);
		void request_quit();

	private:
		void run_mainthread();
		void run_renderthread();
		void run_gamethread();

		void wait_for_renderthread_reaching(const uint64 frame_to_reach, const wait_args& args = {});
		void wait_for_gamethread_reaching(const uint64 frame_to_reach, const wait_args& args = {});

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		std::atomic_bool m_is_quit_requested = false;

		std::thread m_mainthread;
		std::thread m_renderthread;
		std::thread m_gamethread;
		uint64 m_gamethread_frame = 0u;
		uint64 m_renderthread_frame = 0u;

		vector<entity> m_entities{};
		
		run_args m_run_args{};
	};
}


