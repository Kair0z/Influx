#pragma once 
#include "event.h"
#include "core/function.h"
#include "core/basetypes.h"
#include "core/container/containers.h"
#include "core/function.h"

#include <mutex>

namespace influx::events
{
	template <typename ..._evtypes>
	class event_queue final
	{
	public:
		// define an event type and handler
		using my_event			= events::event<_evtypes...>;
		using my_event_handler	= function<void(const my_event&)>;

		event_queue()
		{
			m_eventpool = new event_pool();
		}

		~event_queue()
		{
			delete m_eventpool;
		}

		template <typename _ev, typename _func>
		void subscribe(_func&& handler)
		{
			m_subscribers.push_back([handler](const my_event& generic_event)
			{
				if (_ev const* eventdata = generic_event.get_if<_ev>())
				{
					handler(*eventdata);
				}
			});
		}

		template <typename _ev, class ..._args>
		void push(_args&&... args)
		{
			m_mutex.lock();
			my_event* new_event = m_eventpool->allocate_lockless();
			new_event->set<_ev>(std::forward<_args>(args)...);
			m_eventqueue.push(new_event);
			m_mutex.unlock();
		}

		struct process_args final
		{
			uint32 m_max_num_events = (uint32)-1;
		};

		void process(const process_args& args = {})
		{
			uint32 count = 0u;

			m_mutex.lock();
			if (!m_eventqueue.empty() && count < args.m_max_num_events)
			{
				// pop an event
				my_event* current_event = m_eventqueue.front();
				m_eventqueue.pop();

				// call subscriber callbacks
				for (const my_event_handler& sub : m_subscribers)
				{
					sub(*current_event);
				}

				m_eventpool->free_lockless(current_event);

				++count;
			}
			m_mutex.unlock();
		}

		void nuke_events()
		{
			m_mutex.lock();
			while (!m_eventqueue.empty())
			{
				my_event* ev = m_eventqueue.front();
				m_eventqueue.pop();

				m_eventpool->free_lockless(ev);
			}
			m_mutex.unlock();

			m_eventpool->reset();
		}

		void unsubscribe_all()
		{
			m_subscribers.clear();
		}

		void reset()
		{
			nuke_events();
			unsubscribe_all();
		}

	private:
		vector<my_event_handler> m_subscribers{};
		queue<my_event*> m_eventqueue{};
		std::mutex m_mutex;

		static constexpr uint64 k_ev_capacity = (4096u * 1024u) / sizeof(my_event);
		using event_pool = pool<my_event, k_ev_capacity>;
		event_pool* m_eventpool;
	};
}