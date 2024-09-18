#include "graphics_pch.h"
#include "influx_graphics.h"

#if INFLUX_DX12
#include "influx_graphics/d3d12/dx12_device.h"
#endif
#if INFLUX_VULKAN
#include "influx_graphics/vulkan/vk_device.h"
#endif

namespace influx::graphics
{
	device* device::create(e_api_type type, const device_desc& desc)
	{
		switch (type)
		{
		#if INFLUX_DX12
		case e_api_type::dx12:
			return new dx12_device(desc);
			break;
		#else
			static_assert("NO DX12 4U");
		#endif
		#if INFLUX_VULKAN
		case e_api_type::vulkan:
			return new vk_device(desc); // todo...
			break;
		#else
			static_assert("NO VULKAN 4U");
		#endif
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
		#if INFLUX_DX12
		case e_api_type::dx12:
			break;
		#endif
		default:
			break;
		}

		// setup new:
		switch (type)
		{
		#if INFLUX_DX12
		case e_api_type::dx12:
			break;
		#endif
		default:
			break;
		}
	}
}