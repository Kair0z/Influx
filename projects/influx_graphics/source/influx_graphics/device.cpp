#include "graphics_pch.h"
#include "influx_graphics.h"

#include "influx_graphics/d3d12/dx12_device.h"
#include "influx_graphics/vulkan/vk_device.h"

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
			return new vk_device(); // todo...
			break;

		default:
			// return new null_device();
			return nullptr;
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