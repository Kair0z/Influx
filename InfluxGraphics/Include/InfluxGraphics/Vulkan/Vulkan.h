#pragma once

#ifndef __GR_VULKAN_H_
#define __GR_VULKAN_H_

// Using Core...
#include "Core/BasicTypes.h"
#include "Core/Container/Vector.h"

#include "Vulkan/vulkan.hpp"

namespace Influx::Graphics::Vulkan
{
	inline Vector<String> GetBestExtensions(
		const Vector<vk::ExtensionProperties>& installed,
		const Vector<String>& wanted)
	{
		Vector<String> out_result{};

		for (const String& w : wanted)
		{
			for (vk::ExtensionProperties const& i : installed)
			{
				if (String(i.extensionName).compare(w) == 0)
				{
					out_result.emplace_back(w);
					break;
				}
			}
		}

		return out_result;
	}

	inline vk::Instance CreateVkInstance(const vk::InstanceCreateInfo& createInfo)
	{
		return vk::createInstance(createInfo);
	}

	inline Vector<vk::PhysicalDevice> GetAllVkPhysicalDevices(const vk::Instance instance)
	{
		Vector<vk::PhysicalDevice> out_devices{};
		
		// std::vector -> Influx::Vector
		out_devices = instance.enumeratePhysicalDevices();

		return out_devices;
	}

	inline vk::Device CreateVkLogicalDevice(
		const vk::PhysicalDevice& physDevice,
		const Vector<String>& wantedDeviceExtensions)
	{
		Vector<vk::ExtensionProperties> installedDeviceExtensions =
			physDevice.enumerateDeviceExtensionProperties();

		Vector<String> deviceExtensions 
			= GetBestExtensions(installedDeviceExtensions, wantedDeviceExtensions);

		vk::DeviceCreateInfo dinfo = { {}, queueCreateInfos, deviceExtensions };
		return physDevice.createDevice()
	}
}

#endif