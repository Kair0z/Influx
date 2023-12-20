#include "app_pch.h"
#include "layer_base.h"

namespace influx::application
{
	layer_base::layer_base(const layer_base_args& args)
		: m_base_args{args}
	{
		// default enabled
		set_enabled(true);

		if (is_dedicated_thread())
		{
			m_thread_obj = std::thread([this]()
			{
				vector<e_state> out_deferred_state_changes{};
				while (m_state != e_state::ended)
				{
					// pop all deferred state changes and process them
					out_deferred_state_changes.clear();
					m_deferred_state_changes.pop(out_deferred_state_changes);
					for (e_state new_state : out_deferred_state_changes)
					{
						switch (new_state)
						{
						case e_state::enabled: set_enabled_st(true); break;
						case e_state::disabled: set_enabled_st(false); break;
						}
					}

					process_events_if_enabled_st();
					tick_if_enabled_st();
				}

				set_enabled_st(false);
			});
		}
	}

	layer_base::~layer_base()
	{
		if (is_dedicated_thread())
		{
			defer_state_change(e_state::ended);
		}
	}

	void layer_base::set_enabled(bool new_enabled)
	{
		if (is_dedicated_thread())
		{
			defer_state_change(e_state::enabled);
			return;
		}
		else
		{
			set_enabled_st(new_enabled);
		}
	}

	void layer_base::queue_event(layer_event* e)
	{
		if (!m_event_queue.push(e))
		{

		}
	}

	void layer_base::tick_if_enabled()
	{
		if (!is_dedicated_thread())
		{
			tick_if_enabled_st();
		}
	}

	void layer_base::process_events_if_enabled()
	{
		if (!is_dedicated_thread())
		{
			process_events_if_enabled_st();
		}
	}

	const string& layer_base::get_name() const
	{
		return m_base_args.m_name;
	}

	bool layer_base::is_dedicated_thread() const
	{
		return m_base_args.m_dedicated_thread;
	}

	const layer_base_args& layer_base::get_base_args() const
	{
		return m_base_args;
	}

	void layer_base::defer_state_change(e_state new_state)
	{
		m_deferred_state_changes.push(new_state);
	}

	void layer_base::set_enabled_st(bool new_enabled)
	{
		if (new_enabled && m_state != e_state::enabled)
		{
			on_enable();
			m_state = e_state::enabled;
			return;
		}

		if (!new_enabled && m_state == e_state::enabled)
		{
			on_disable();
			m_state = e_state::disabled;
			return;
		}
	}

	void layer_base::tick_if_enabled_st()
	{
		if (m_state == e_state::enabled)
		{
			on_tick();
		}
	}

	void layer_base::process_events_if_enabled_st()
	{
		if (m_state != e_state::enabled)
		{
			return;
		}

		if (m_event_queue.is_empty())
		{
			return;
		}

		// pop all events
		vector<layer_event*> out_events{};
		m_event_queue.pop(out_events);

		for (layer_event* e : out_events)
		{
			on_event(e);
		}
	}
}

