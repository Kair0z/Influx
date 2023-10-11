#pragma once

#ifndef __GR_VULKAN_H_
#define __GR_VULKAN_H_

#include "Core/Container/Vector.h"

// https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp

#include "Vulkan/vulkan.hpp"

// Set to "true" to enable Vulkan's validation layers (see vulkandebug.cpp for details)
#define ENABLE_VALIDATION false

// Set to "true" to use staging buffers for uploading vertex and index data to device local memory
// See "prepareVertices" for details on what's staging and on why to use it
#define USE_STAGING true

namespace influx::Graphics::Vulkan
{
	// [HELPERS]
	inline Vector<const char*> FilterWantedAndInstalledExtensions(
		const Vector<vk::ExtensionProperties>& installed,
		const Vector<const char*>& wanted)
	{
		Vector<const char*> out{};

		for (const auto& w : wanted)
		{
			for (const vk::ExtensionProperties& i : installed)
			{
				if (std::string(i.extensionName.m_data()).compare(w) == 0)
				{
					out.push_back(w);
					break;
				}
			}
		}

		return out;
	}

	inline Vector<const char*> FilterWantedAndInstalledLayers(
		const Vector<vk::LayerProperties>& installed,
		const Vector<const char*>& wanted)
	{
		Vector<const char*> out{};

		for (const auto& w : wanted)
		{
			for (const vk::LayerProperties& i : installed)
			{
				if (std::string(i.layerName.m_data()).compare(w) == 0)
				{
					out.push_back(w);
					break;
				}
			}
		}

		return out;
	}

	inline uint32 GetQueueIndex(const vk::PhysicalDevice& physicalDevice, vk::QueueFlagBits flags)
	{
		Vector<vk::QueueFamilyProperties> queueProps =
			physicalDevice.getQueueFamilyProperties();

		for (uint64 i = 0; i < queueProps.dimension(); ++i)
		{
			if (queueProps[i].queueFlags & flags)
			{
				return static_cast<uint32>(i);
			}
		}

		// Default queue index
		return 0u;
	}

	inline uint32 GetMemoryTypeIndex(const vk::PhysicalDevice& physicalDevice, uint32 typeBits, vk::MemoryPropertyFlags properties)
	{
		auto gpuMemoryProps = physicalDevice.getMemoryProperties();
		for (uint32 i = 0; i < gpuMemoryProps.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((gpuMemoryProps.memoryTypes[i].propertyFlags & properties) ==
					properties)
				{
					return i;
				}
			}
			typeBits >>= 1;
		}

		return 0u;
	};

	// 
	inline vk::Instance CreateVkInstance(const vk::InstanceCreateInfo& info)
	{
		return vk::createInstance(info);
	}

	inline vk::Instance CreateVkInstance(const vk::ApplicationInfo& appInfo,
		const Vector<const char*>& wantedExtensions = {},
		const Vector<const char*>& wantedLayers = {},
		vk::InstanceCreateFlags createFlags = vk::InstanceCreateFlags())
	{
		std::vector<vk::ExtensionProperties> installedExtensions = vk::enumerateInstanceExtensionProperties();
		std::vector<const char*> resultInstanceExtensions = FilterWantedAndInstalledExtensions(installedExtensions, wantedExtensions);

		std::vector<vk::LayerProperties> installedLayers = vk::enumerateInstanceLayerProperties();
		std::vector<const char*> resultInstanceLayers = FilterWantedAndInstalledLayers(installedLayers, wantedLayers);

		vk::InstanceCreateInfo ci = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(), &appInfo, resultInstanceLayers, resultInstanceExtensions);

		return vk::createInstance(ci);
	}

	inline Vector<vk::PhysicalDevice> GetAllVkPhysicalDevices(const vk::Instance& instance)
	{
		uint32_t numPhysicalDevices = 0u;
		auto result = instance.enumeratePhysicalDevices(&numPhysicalDevices, nullptr);

		if (numPhysicalDevices > 0u)
		{
			Vector<vk::PhysicalDevice> devices(numPhysicalDevices);
			auto result = instance.enumeratePhysicalDevices(&numPhysicalDevices, devices.m_data());
			return devices;
		}
		else
		{
			return {};
		}
	}

	inline vk::Device CreateVkLogicalDevice(const vk::PhysicalDevice& parentDevice,
		const Vector<vk::DeviceQueueCreateInfo>& queuesToCreate = {},
		const Vector<const char*>& wantedExtensions = {},
		const Vector<const char*>& wantedLayers = {},
		vk::DeviceCreateFlags flags = vk::DeviceCreateFlags())
	{
		std::vector<vk::ExtensionProperties> installedExtensions = vk::enumerateInstanceExtensionProperties();
		std::vector<const char*> resultDeviceExtensions = FilterWantedAndInstalledExtensions(installedExtensions, wantedExtensions);

		std::vector<vk::LayerProperties> installedLayers = vk::enumerateInstanceLayerProperties();
		std::vector<const char*> resultDeviceLayers = FilterWantedAndInstalledLayers(installedLayers, wantedLayers);

		vk::DeviceCreateInfo dinfo = { flags, queuesToCreate,
			resultDeviceLayers, resultDeviceExtensions };

		return parentDevice.createDevice(dinfo);
	}

	inline vk::Queue CreateVkGraphicsQueue(const vk::Device& parentDevice)
	{
		vk::DeviceQueueCreateInfo queueInfo{};
		return {};
	}
}

#endif