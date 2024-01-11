#include "graphics_pch.h"
#include "influx_graphics.h"

#include "influx_graphics/d3d12/dx12_graphics.h"
#include "influx_graphics/vulkan/vulkan_graphics.h"
#include "influx_graphics/null/null_graphics.h"

namespace influx::graphics
{
	device* device::create(e_api_type type)
	{
		switch (type)
		{
		case e_api_type::dx12:
			return new dx12_device();
			break;

		case e_api_type::vulkan:
			return new vulkan_device();
			break;

		default:
			return new null_device();
			break;
		}
	}

	void device::set_api_type(e_api_type type)
	{
		// handle previous?
		switch (m_type)
		{
		case e_api_type::dx12:
			break;
		default:
			break;
		}

		// setup new:
		switch (type)
		{
		case e_api_type::dx12:
			break;
		default:
			break;
		}
	}
}