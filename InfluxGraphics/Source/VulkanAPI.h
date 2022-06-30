#pragma once

#ifndef _VULKAN_API_H_
#define _VULKAN_API_H_

#include "GraphicsAPI.h"

#include "Vulkan/vulkan.hpp"

namespace Influx::Graphics
{
	/* VulkanAPI -> RHI */
	class VulkanAPI final : public GraphicsAPI
	{
		/* Private constructor -> Singleton */
		VulkanAPI();

	public:
		/* Singleton Object holding references to ID3D12Device, IDXGIFactory, ... */
		static VulkanAPI& Get()
		{
			static VulkanAPI api{};
			return api;
		}
		virtual ~VulkanAPI();

		
	};
}

#endif