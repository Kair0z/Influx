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
	constexpr static uint8 k_cache_line_num_bytes = 64u;
	constexpr static bool k_render_scene = true;
	constexpr static bool k_jobify = true;
	constexpr static uint8 k_max_num_job_threads = 4u;
	constexpr static uint64 k_num_entities = 10u;
	constexpr static bool k_force_vsync = false;
	constexpr static bool k_force_single_threaded = false;

	constexpr static uint64 k_stats_capacity = 256u;
	constexpr static uint64 k_stats_log_frame_intv = k_stats_capacity;
	
	enum class e_dedicated_thread : uint8
	{
		gamethread,
		renderthread,
		max
	};

	struct per_frame_stats final
	{
		float m_ms_total = 0.0f;	// total ms frame
		float m_pc_sync = 0.0f;		// percentage of total ms spent on syncing

		per_frame_stats& operator+=(const per_frame_stats& other)
		{
			m_pc_sync += other.m_pc_sync;
			m_ms_total += other.m_ms_total;
			return *this;
		}
		per_frame_stats& operator/=(const float& div)
		{
			m_pc_sync /= div;
			m_ms_total /= div;
			return *this;
		}
	};

	class gamethread;
	class renderthread;
	class dedicated_thread;

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
		static bool is_single_threaded();
		static bool is_vsync();
		static bool is_editor_enabled();
		static bool is_commandlet();

		static per_frame_stats get_average_frame_stats(e_dedicated_thread thread);

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

		const dedicated_thread* find_thread(e_dedicated_thread thread_type) const;
	};
}


