#include "graphics_pch.h"
#include "influx_graphics/resource_views.h"

namespace influx::graphics
{
	descriptor_handle resource_view::get_descriptor_handle() const
	{
		return m_handle;
	}
}

