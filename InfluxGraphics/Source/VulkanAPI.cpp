#include "VulkanAPI.h"

namespace Influx::Graphics
{
	VulkanAPI::VulkanAPI()
	{
		CreateInstance(VkInstance, "None");

		MainDevice = VulkanDevice(PickFirstSuitablePhysicalDevice(VulkanAPI::GetPhysicalDevices(VkInstance)));
		MainDevice.CreateLogicalDevice({}, {}, true);
	}

	VulkanAPI::~VulkanAPI()
	{
		vkDestroyInstance(VkInstance, nullptr);
	}

	vk::PhysicalDevice VulkanAPI::PickFirstSuitablePhysicalDevice(const std::vector<vk::PhysicalDevice>& devices)
	{
		vk::PhysicalDevice result = VK_NULL_HANDLE;

		for (int i = 0; i < devices.size(); ++i)
		{
			// Basic device properties like the name, type and supported Vulkan version can be queried using vkGetPhysicalDeviceProperties.
			vk::PhysicalDeviceProperties deviceProperties = devices[i].getProperties();

			// The support for optional features like texture compression, 64 bit floats and multi viewport rendering (useful for VR) can be queried using vkGetPhysicalDeviceFeatures:
			vk::PhysicalDeviceFeatures deviceFeatures = devices[i].getFeatures();		
			
			// Todo: Make this less arbitrary :), even better, pick between gpu's based on a 'suitability-score'...
			const bool isSuitable = deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader;
			if (isSuitable)
			{
				result = devices[i];
				break;
			}
		}

		return result;
	}

	void VulkanAPI::CreateInstance(vk::Instance& outResult, bool validation, const std::string& appName)
	{
		vk::Instance instance{};

		vk::ApplicationInfo appInfo{};
		appInfo.sType = vk::StructureType::eApplicationInfo;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eInstanceCreateInfo;
		createInfo.pApplicationInfo = &appInfo;

		// Additional Extensions...
		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<vk::ExtensionProperties> extensions(extensionCount);
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		char** extensionNames = new char* [extensionCount] {};
		for (int i = 0; i < extensionCount; ++i)
		{
			extensionNames[i] = extensions[i].extensionName;
		}
		createInfo.enabledExtensionCount = extensionCount;
		createInfo.ppEnabledExtensionNames = extensionNames;

		// Validation Layers...
#if _DEBUG
		if (validation)
		{
			const std::vector<const char*>& validationLayers =
			{
				// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
				// Note that on Android this layer requires at least NDK r20
				"VK_LAYER_KHRONOS_validation"
			};

			if (!CheckValidationLayerSupport(validationLayers))
			{
				// One of the requested validationLayers is not available!...
				assert(false);
			}

			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
#else
		createInfo.enabledLayerCount = 0;
#endif
		
		vk::Result result = vk::createInstance(&createInfo, nullptr, &outResult);

		delete[] extensionNames;
	}

	std::vector<vk::PhysicalDevice> VulkanAPI::GetPhysicalDevices(const vk::Instance& instance)
	{
		return instance.enumeratePhysicalDevices();
	}

	vk::Device VulkanAPI::CreateLogicalDeviceAndQueues(vk::PhysicalDevice physicalDevice, std::vector<vk::Queue>& outQueues)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
		queueCreateInfo.queueFamilyIndex = 0;
		queueCreateInfo.queueCount = 1;
		float queuePrio = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePrio;

		// For now, no fancy features necessary...
		vk::PhysicalDeviceFeatures deviceFeatures{};

		vk::DeviceCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eDeviceCreateInfo;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;
		createInfo.enabledLayerCount = 0;

		vk::Device result = physicalDevice.createDevice(createInfo);

		result.getQueue(0, 0, &outQueues[0]);
		return result;
	}

	bool VulkanAPI::CheckValidationLayerSupport(const std::vector<const char*>& validationLayerNames)
	{
		uint32_t layerCount;
		vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<vk::LayerProperties> layers(layerCount);
		vk::enumerateInstanceLayerProperties(&layerCount, layers.data());

		for (int i = 0; i < validationLayerNames.size(); ++i)
		{
			const char* layerNameToCheck = validationLayerNames[i];

			bool layerFound = false;
			for (int l = 0; l < layers.size(); ++l)
			{
				if (std::strcmp(layerNameToCheck, layers[l].layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return false;
	}

	vk::CommandBuffer VulkanCommandList::GetVulkanCommandBuffer() const
	{
		return VkCommandBuffer;
	}

	vk::Queue VulkanCommandQueue::GetVulkanQueue() const
	{
		return VkCommandQueue;
	}

	VulkanCommandQueue::~VulkanCommandQueue()
	{
		
	}
	
	VulkanAPI::VulkanDevice::VulkanDevice(vk::PhysicalDevice physicalDevice)
	{
		VkPhysicalDevice = physicalDevice;

		VkProperties = physicalDevice.getProperties();
		VkFeatures = physicalDevice.getFeatures();
		VkMemoryProperties = physicalDevice.getMemoryProperties();
		VkQueueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	}

	VulkanAPI::VulkanDevice::~VulkanDevice()
	{
		if (VkLogicalDevice) vkDestroyDevice(VkLogicalDevice, nullptr);
	}

	void VulkanAPI::VulkanDevice::CreateLogicalDevice(vk::PhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain, vk::QueueFlags requestedQueueTypes)
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & vk::QueueFlagBits::eGraphics)
		{
			QueueFamilyIndices.Graphics = GetQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = QueueFamilyIndices.Graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			QueueFamilyIndices.Graphics = 0;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & vk::QueueFlagBits::eCompute)
		{
			QueueFamilyIndices.Compute = GetQueueFamilyIndex(vk::QueueFlagBits::eCompute);
			if (QueueFamilyIndices.Compute != QueueFamilyIndices.Graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = QueueFamilyIndices.Compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			QueueFamilyIndices.Compute = QueueFamilyIndices.Graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & vk::QueueFlagBits::eTransfer)
		{
			QueueFamilyIndices.Transfer = GetQueueFamilyIndex(vk::QueueFlagBits::eTransfer);
			if ((QueueFamilyIndices.Transfer != QueueFamilyIndices.Graphics) && (QueueFamilyIndices.Transfer != QueueFamilyIndices.Compute))
			{
				// If transfer family index differs, we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = QueueFamilyIndices.Transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			QueueFamilyIndices.Transfer = QueueFamilyIndices.Graphics;
		}

		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain)
		{
			// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		vk::DeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		VkEnabledFeatures = enabledFeatures;

		VkLogicalDevice = VkPhysicalDevice.createDevice(deviceCreateInfo, nullptr);
	}

	uint32_t VulkanAPI::VulkanDevice::GetQueueFamilyIndex(vk::QueueFlags queueFlags) const
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if ((queueFlags & vk::QueueFlagBits::eCompute) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(VkQueueFamilyProperties.size()); i++)
			{
				if ((VkQueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute) && 
					(!(VkQueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)))
				{
					return i;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if ((queueFlags & vk::QueueFlagBits::eTransfer) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(VkQueueFamilyProperties.size()); i++)
			{
				if ((VkQueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer) && 
					(!(VkQueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)) && 
					(!(VkQueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)))
				{
					return i;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(VkQueueFamilyProperties.size()); i++)
		{
			if ((VkQueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	vk::Queue VulkanAPI::VulkanDevice::RequestDeviceQueue(vk::QueueFlagBits queueType, uint32_t queueIndex)
	{
		return VkLogicalDevice.getQueue(GetQueueFamilyIndex(queueType), queueIndex);
	}

}

