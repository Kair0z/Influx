#include "events_pch.h"
#include "influx_events.h"

namespace influx::events
{
	event_queue::event_queue(const event_queue_init& init)
	{
		m_eventpool = new event_pool();
	}

	event_queue::~event_queue()
	{
		delete m_eventpool;
	}

	void event_queue<..._evtypes>::subscribe(event_handler callback)
	{
		return INFLUX_EVENTS_API void();
	}

	void event_queue::subscribe(event_callback callback)
	{
		m_subscribers.push_back(callback);
	}

	void event_queue::push(const event& ev)
	{
		m_mutex.lock();
		m_eventqueue.push(m_eventpool->allocate_lockless());
		m_mutex.unlock();
	}

	void event_queue::service(const service_args& args)
	{
		uint32 count = 0u;

		m_mutex.lock();
		if (!m_eventqueue.empty() && count < args.m_max_num_events)
		{
			// pop an event
			event* current_event = m_eventqueue.front();
			m_eventqueue.pop();

			// call subscriber callbacks
			for (event_callback sub : m_subscribers)
			{
				sub(current_event);
			}

			m_eventpool->free_lockless(current_event);

			++count;
		}
		m_mutex.unlock();
	}

	// - clear queue
	// - removes all subscribers
	// - reset event pool
	void event_queue::reset()
	{
		nuke_events();
		unsubscribe_all();
	}

	void event_queue::nuke_events()
	{
		m_mutex.lock();
		while (!m_eventqueue.empty())
		{
			event* ev = m_eventqueue.front();
			m_eventqueue.pop();

			m_eventpool->free_lockless(ev);
		}
		m_mutex.unlock();

		m_eventpool->reset();
	}

	void event_queue::unsubscribe_all()
	{
		m_subscribers.clear();
	}
}