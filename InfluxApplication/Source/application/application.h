#pragma once

#include "influx_application.h"

#include "core/singleton/singleton.h"
#include "core/platform/platform.h"
#include "Core/Container/Vector.h"
#include "core/Math/Matrix.h"
#include "Core/Math/Transform.h"
#include "Core/Container/RingBuffer.h"

#include "application/threads/rendersync.h"

#include <atomic>
#include <thread>

namespace influx::renderer
{
	struct material_data;
	struct scene_proxy;
}

namespace influx::application
{
	constexpr static bool k_render_scene = true;
	constexpr static bool k_jobify = true;
	constexpr static uint8 k_max_num_job_threads = 8u;
	constexpr static uint64 k_stats_capacity = 512u;
	constexpr static uint64 k_num_entities = 512u;
	constexpr static uint64 k_stats_log_frame_intv = 512u;
	constexpr static uint64 k_num_stats_to_average = 64u;

	class dedicated_thread;
	class gamethread;
	class renderthread;

	class application final
		: public singleton<application>
	{
	public:
		void run(const run_args& args);
		void request_quit();

		string get_resource_directory() const;
		run_args get_run_arguments() const;

		platform::window_handle get_window_handle() const;
		platform::instance_handle get_instance_handle() const;
		static rendersync& get_render_sync();

		static bool is_quit_requested();

	private:
		void main_init();
		void main_tick();
		void main_cleanup();
		void mainthread_log();
		uint64 m_mainthread_frame = 0u;

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;
		std::atomic_bool m_is_quit_requested = false;

		vector<dedicated_thread*> m_dedicated_threads{};
		gamethread* mp_gamethread = nullptr;
		renderthread* mp_renderthread = nullptr;

		rendersync m_render_sync{};
		run_args m_run_args{};
	};
}


