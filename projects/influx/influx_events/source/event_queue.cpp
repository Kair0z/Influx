#include "events_pch.h"
#include "influx_events.h"

namespace influx::events
{
	event_queue::event_queue(const event_queue_init& init)
	{

	}

	void event_queue::subscribe(event_callback callback)
	{
		m_subscribers.push_back(callback);
	}

	void event_queue::push(const event& ev)
	{
		m_mutex.lock();
		m_events.push(ev);
		m_mutex.unlock();
	}

	void event_queue::service(const service_args& args)
	{
		uint32 count = 0u;

		m_mutex.lock();
		if (!m_events.empty() && count < args.m_max_num_events)
		{
			// pop an event
			const event& ev = m_events.front();
			m_events.pop();

			// call subscriber callbacks
			for (event_callback sub : m_subscribers)
			{
				sub(ev);
			}

			++count;
		}
		m_mutex.unlock();
	}

	// nukes all events, and removes all subscribers!
	void event_queue::reset()
	{
		nuke_events();
		unsubscribe_all();
	}

	void event_queue::nuke_events()
	{
		m_mutex.lock();

		while (!m_events.empty())
			m_events.pop();

		m_mutex.unlock();
	}

	void event_queue::unsubscribe_all()
	{
		m_subscribers.clear();
	}
}