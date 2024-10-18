#pragma once

#if _DLL
#define INFLUX_EVENTS_API __declspec(dllexport)
#else
#define INFLUX_EVENTS_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/container/containers.h"
#include "core/function.h"

#include "core/events/event_queue.h"

#include <mutex>
#include <variant>

namespace influx::events
{
	namespace detail
	{
		class ievent
		{
		public:

		};
	}
	
	template <typename ..._types>
	class event final : public ievent
	{
	public:
		INFLUX_EVENTS_API event(void* mp_data);
		INFLUX_EVENTS_API void* get_data() const;

	private:
		std::variant<_types> m_variant_data;
	};

	struct event_queue_init final
	{
		uint64 m_min_budget_in_bytes	= 4096u * 1024u; // 4KB
		uint64 m_max_budget_in_bytes	= 4096u * 1024u; // 4MB
	};

	template <typename ..._evtypes>
	class event_queue final
	{
	public:
		INFLUX_EVENTS_API event_queue(const event_queue_init & = {});
		INFLUX_EVENTS_API ~event_queue();

		using event_handler = function<void(const event<_evtypes...>& ev)>;
		INFLUX_EVENTS_API void subscribe(event_handler callback);

		// pushes events into the queue
		INFLUX_EVENTS_API void push(const event& ev);

		// pops events off the queue
		struct service_args final
		{
			uint32 m_max_num_events = (uint32)-1;
		};
		INFLUX_EVENTS_API void service(const service_args& args = {});

		// nukes all events, and removes all subscribers!
		INFLUX_EVENTS_API void reset();
		INFLUX_EVENTS_API void nuke_events();
		INFLUX_EVENTS_API void unsubscribe_all();

	private:
		vector<event_callback> m_subscribers{};
		queue<event*> m_eventqueue{};
		std::mutex m_mutex;

		static constexpr uint64 k_event_capacity = (4096u * 1024u) / sizeof(events::event);
		using event_pool = pool<events::event<_evtypes...>, k_event_capacity>;
		event_pool* m_eventpool;
	};
}