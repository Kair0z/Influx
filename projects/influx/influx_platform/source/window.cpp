#include "influx_platform/window.h"

namespace influx::platform
{
	window::window(const window_desc& desc)
		: m_desc{desc}
	{

	}

	void window::request_quit()
	{
		m_has_quit_event = true;
	}

	bool window::has_quit_request() const
	{
		return m_has_quit_event;
	}
}