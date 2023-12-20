#include "app_pch.h"
#include "layer_base.h"

namespace influx::application
{
	layer_base::layer_base(const layer_base_args& args)
		: m_base_args{args}
	{
		set_enabled(true);
	}

	layer_base::~layer_base()
	{
		set_enabled(false);
	}

	void layer_base::set_enabled(bool new_enabled)
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

	void layer_base::queue_event(layer_event* e)
	{
		influx_assert(m_event_queue.push(e));
	}

	void layer_base::tick_if_enabled()
	{
		if (m_state == e_state::enabled)
		{
			on_tick();
		}
	}

	void layer_base::process_events_if_enabled()
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

	const string& layer_base::get_name() const
	{
		return m_base_args.m_name;
	}

	const layer_base_args& layer_base::get_base_args() const
	{
		return m_base_args;
	}
}

