#pragma once

#include "influx_application.h"

#include "core/singleton/singleton.h"
#include "core/platform/platform.h"
#include "Core/Container/Vector.h"
#include "core/Math/Matrix.h"
#include "Core/Math/Transform.h"
#include "Core/Container/RingBuffer.h"

#include <atomic>
#include <thread>

namespace influx::renderer
{
	struct material_data;
}

namespace influx::application
{
	class application final 
		: public singleton<application>
	{
		struct wait_args final
		{
			wait_args() = default;
			wait_args(float* out_seconds_waited) : mp_out_seconds_waited{ out_seconds_waited } {}
			float* mp_out_seconds_waited = nullptr;
		};
		struct entity final
		{
			entity() = default;
			entity(uint64 id) : m_id{ id } {}

			uint64 m_id = 0u;
			math::transform3D m_transform = math::transform3D::identity();
		};
		struct frame_stats final
		{
			float m_pc_sync = 0.0f;
			float m_ms_total = 0.0f;

			frame_stats& operator+=(const frame_stats& other)
			{
				m_pc_sync += other.m_pc_sync;
				m_ms_total += other.m_ms_total;
				return *this;
			}

			frame_stats& operator/=(const float& div)
			{
				m_pc_sync	/= div;
				m_ms_total	/= div;
				return *this;
			}
		};
		struct thread_state final
		{
			ringbuffer<frame_stats, 256u> m_stats{};
		};

	public:
		void run(const run_args& args);
		void request_quit();

	private:
		void run_mainthread();
		void run_renderthread();
		void run_gamethread();
		void run_editorthread();

		void wait_for_renderthread_reaching(const uint64 frame_to_reach, const wait_args& args = {});
		void wait_for_gamethread_reaching(const uint64 frame_to_reach, const wait_args& args = {});

		void mainthread_log();

		void renderthread_loadassets(uint32& num_submeshes, vector<renderer::material_data>& materials);

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		std::atomic_bool m_is_quit_requested = false;

		std::thread m_mainthread;
		std::thread m_renderthread;
		std::thread m_gamethread;
		std::thread m_editorthread;

		thread_state m_gamethread_state{};
		thread_state m_renderthread_state{};

		uint64 m_gamethread_frame = 0u;
		uint64 m_renderthread_frame = 0u;

		vector<entity> m_entities{};
		entity m_camera_entity{};

		run_args m_run_args{};
	};
}


