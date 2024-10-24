#include "rhi_pch.h"
#include "influx_rhi.h"

#if INFLUX_RHI_DX12
#include "influx_rhi/d3d12/d3d12_layer.h"
#endif

#if INFLUX_RHI_VULKAN
#include "influx_rhi/vulkan/vulkan_layer.h"
#endif

#include "influx_rhi/null/null_layer.h"

namespace influx::rhi
{
	device* device::create(e_api_type api)
	{
		switch (api)
		{
#if INFLUX_RHI_DX12
		case e_api_type::dx12: return new dx12_device();
#endif

#if INFLUX_RHI_VULKAN
		case e_api_type::vulkan: return new vulkan_device();
#endif

		default:
		case e_api_type::null: return new null_device();
		}
	}
}