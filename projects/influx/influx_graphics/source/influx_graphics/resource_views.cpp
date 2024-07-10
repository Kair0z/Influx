#include "graphics_pch.h"
#include "influx_graphics/resource_views.h"

namespace influx::graphics
{
	descriptor_handle resource_view::get_cpu_handle() const
	{
		return m_cpu_handle;
	}

	descriptor_handle resource_view::get_gpu_handle() const
	{
		return m_gpu_handle;
	}
}

