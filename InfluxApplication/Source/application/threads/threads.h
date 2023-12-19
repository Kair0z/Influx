#pragma once

#include "application/application_backend.h"
#include <thread>

namespace influx::application
{
	class dedicated_thread
	{
	public:
		inline virtual ~dedicated_thread()
		{
			if (m_thread_object.joinable())
				m_thread_object.join();
		}

		virtual e_dedicated_thread get_thread_type() const = 0;

		// spins a looping run into a new thread object
		inline void spin()
		{
			m_thread_object = std::thread([this]()
			{
				call_initialize();
				while (application::is_quit_requested() == false)
				{
					call_tick();
				}
				call_cleanup();
			});
		};
		inline void call_initialize()
		{
			m_time_before_init = time::get_now();
			initialize();
			m_time_after_init = time::get_now();
			m_is_initialized = true;
		}
		inline void call_tick()
		{
			m_time_before_tick = m_time_before_sync = m_time_after_sync = time::get_now();
			tick();
			m_time_after_tick = time::get_now();

			// record stats of this tick
			per_frame_stats this_frame_stats{};
			const float ms_sync = time::get_ms_between<float>(m_time_after_sync, m_time_before_sync);
			this_frame_stats.m_ms_total = time::get_ms_between<float>(m_time_after_tick, m_time_before_tick);
			this_frame_stats.m_pc_sync = math::is_zero(this_frame_stats.m_ms_total) ? 0.0f : ms_sync / this_frame_stats.m_ms_total;
			m_stats.pop_to_push(this_frame_stats);

			++m_frame;
		}
		inline void call_cleanup()
		{
			cleanup();
		}

		inline per_frame_stats calc_average_stats()
		{
			m_last_average_stats = m_stats.get_average_value();
			return m_last_average_stats;
		}

		inline uint64 get_frame() const
		{
			return m_frame;
		}

		inline float get_ms_initialize() const
		{
			return time::get_ms_between<float>(m_time_after_init, m_time_before_init);
		}

		inline const per_frame_stats& get_average_stats() const
		{
			return m_last_average_stats;
		}

		void wait_for_initialize()
		{
			while (!m_is_initialized)
			{

			}
		}

	protected:
		inline void mark_sync_start()
		{
			m_time_before_sync = time::get_now();
		}
		inline void mark_sync_end()
		{
			m_time_after_sync = time::get_now();
		}

	private:
		std::thread m_thread_object{};
		uint64 m_frame = 0u;
		ringbuffer<per_frame_stats, k_stats_capacity> m_stats{};
		per_frame_stats m_last_average_stats{};

		time::point m_time_before_init{};
		time::point m_time_after_init{};
		time::point m_time_before_tick{};
		time::point m_time_after_tick{};
		time::point m_time_before_sync{};
		time::point m_time_after_sync{};

		bool m_is_initialized = false;

		virtual void initialize() = 0;
		virtual void tick() = 0;
		virtual void cleanup() = 0;
	};
}