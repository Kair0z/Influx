#pragma once

#if _DLL
#define INFLUX_EVENTS_API __declspec(dllexport)
#else
#define INFLUX_EVENTS_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/container/containers.h"
#include "core/function.h"

#include <thread>
#include <mutex>

namespace influx::events
{
	class event final
	{
	public:
		INFLUX_EVENTS_API event(void* mp_data);
		INFLUX_EVENTS_API void* get_data() const;

	private:
		void* mp_data;
	};

	struct event_queue_init final
	{
		uint32 m_num_threads = 2u;
	};

	class event_queue final
	{
	public:
		enum class e_state : uint8
		{
			active,
			paused,
			count
		};
		constexpr static uint32 k_max_num_threads = 8u;

	public:
		INFLUX_EVENTS_API event_queue(const event_queue_init & = {});
		INFLUX_EVENTS_API ~event_queue() = default;

		using event_callback = function<void(const event& ev)>;
		INFLUX_EVENTS_API void subscribe(event_callback callback);

		INFLUX_EVENTS_API void push(const event& ev);

		// nukes all events, and removes all subscribers!
		INFLUX_EVENTS_API void reset();
		INFLUX_EVENTS_API void nuke_events();
		INFLUX_EVENTS_API void unsubscribe_all();

	private:
		vector<event_callback> m_subscribers{};
		queue<event> m_events{};
		std::thread m_workers[k_max_num_threads]{};
		std::mutex m_mutex;
		e_state m_state = e_state::active;

		void worker_loop();
	};
}