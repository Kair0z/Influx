#pragma once

#include <thread>
#include "Core/Container/RingBuffer.h"

namespace influx::application
{
	constexpr static uint64 k_stats_capacity = 512u;

#define def_inherit_static_void_func(func) \
	public: \
		static void static_##func; \
	private: \
		inline virtual void call_##func override { static_##func; }; \

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

		virtual ~dedicated_thread();

		// spins implementation off onto a new thread object
		void spin();

	private:
		std::thread m_thread_object{};
		uint64 m_frame = 0u;
		ringbuffer<per_frame_stats, k_stats_capacity> m_stats{};

		virtual void call_initialize() = 0;
		virtual void call_tick() = 0;
		virtual void call_cleanup() = 0;
	};

	class renderthread final : public dedicated_thread
	{
	public:
		def_inherit_static_void_func(initialize());
		def_inherit_static_void_func(tick());
		def_inherit_static_void_func(cleanup());
	};

	class gamethread final : public dedicated_thread
	{
	public:
		def_inherit_static_void_func(initialize());
		def_inherit_static_void_func(tick());
		def_inherit_static_void_func(cleanup());
	};

	class mainthread final : public dedicated_thread
	{
	public:
		def_inherit_static_void_func(initialize());
		def_inherit_static_void_func(tick());
		def_inherit_static_void_func(cleanup());
	};
}