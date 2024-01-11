#include "renderer_pch.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

namespace influx::renderer
{
	target::target(graphics::device* device, const platform::window_handle& from_window)
		: target(device, make_from_window(from_window))
	{
		
	}

	target::target(graphics::device* device, const target_create_args& args)
	{
		// create the resource
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions;
		desc.m_flags;
		desc.m_format;
		desc.m_num_mips;
		desc.m_sample_count;

		mp_resource = device->create_resource(desc);
	}

	target_create_args target::make_from_window(const platform::window_handle& window)
	{
		target_create_args result{};
		return result;
	}
}

