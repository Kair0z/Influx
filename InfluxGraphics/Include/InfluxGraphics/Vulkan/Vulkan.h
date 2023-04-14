#pragma once

#ifndef __GR_D3D12_H_
#define __GR_D3D12_H_

#include "Core/Container/Vector.h"

// https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp

#include "Vulkan/vulkan.hpp"

// Set to "true" to enable Vulkan's validation layers (see vulkandebug.cpp for details)
#define ENABLE_VALIDATION false

// Set to "true" to use staging buffers for uploading vertex and index data to device local memory
// See "prepareVertices" for details on what's staging and on why to use it
#define USE_STAGING true

namespace Influx::Graphics::Vulkan
{
	inline vk::Instance CreateVkInstance(const vk::InstanceCreateInfo& info)
	{
		return vk::createInstance(info);
	}

	inline Vector<vk::PhysicalDevice> GetAllVkPhysicalDevices(const vk::Instance& instance)
	{
		uint32_t numPhysicalDevices = 0u;
		instance.enumeratePhysicalDevices(&numPhysicalDevices, nullptr);

		if (numPhysicalDevices > 0u)
		{
			Vector<vk::PhysicalDevice> devices(numPhysicalDevices);
			instance.enumeratePhysicalDevices(&numPhysicalDevices, devices.data());
			return devices;
		}
		else
		{
			return {};
		}
	}

	inline vk::Device CreateVkDevice(const vk::PhysicalDevice& parentDevice, const vk::DeviceCreateInfo& info)
	{
		return parentDevice.createDevice(info);
	}

	inline vk::Queue CreateVkGraphicsQueue(const vk::Device& parentDevice)
	{
		vk::DeviceQueueCreateInfo queueInfo{};
		return {};
	}
}

#endif