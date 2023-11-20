#pragma once

#include "application/application.h"
#include "Core/Container/ringbuffer.h"
#include "Core/Time.h"

#include <thread>

namespace influx::application
{
	enum class e_dedicated_thread : uint8
	{
		gamethread,
		renderthread,
		max
	};

	class dedicated_thread
	{
	public:
		struct per_frame_stats final
		{
			float m_pc_sync = 0.0f;
			float m_ms_total = 0.0f;

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

		inline virtual ~dedicated_thread()
		{
			if (m_thread_object.joinable())
				m_thread_object.join();
		}

		// spins implementation off onto a new thread object
		inline void spin()
		{
			m_thread_object = std::thread([this]()
			{
				m_time_before_init = time::get_now();
				initialize();
				m_time_after_init = time::get_now();
				while (true)
				{
					m_time_before_tick = time::get_now();
					tick();
					m_time_after_tick = time::get_now();
				}
				cleanup();
			});
		}

		virtual void initialize() = 0;
		virtual void tick() = 0;
		virtual void cleanup() = 0;
		virtual e_dedicated_thread get_thread_type() const = 0;

		inline per_frame_stats get_average_stats(uint64 num_elements)
		{
			return m_stats.get_average_value(num_elements);
		}

	private:
		std::thread m_thread_object{};
		uint64 m_frame = 0u;
		ringbuffer<per_frame_stats, k_stats_capacity> m_stats{};

		time::point m_time_before_init{};
		time::point m_time_after_init{};
		time::point m_time_before_tick{};
		time::point m_time_after_tick{};
	};
}