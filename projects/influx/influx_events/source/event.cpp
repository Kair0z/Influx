#include "events_pch.h"
#include "influx_events.h"

namespace influx::events
{
	event::event(void* data)
		: mp_data{data}
	{

	}

	void* event::get_data() const
	{
		return mp_data;
	}
}