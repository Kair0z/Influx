#include "VulkanAPI.h"
#include "VulkanConversion.h"

#include <iostream>

namespace Influx::Graphics
{
	// Debug API call...
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanAPI::debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*)
	{
		return false;
	}

	VulkanAPI::VulkanAPI()
	{
		CreateInstance(VulkInstance, "None");

		MainDevice = VulkanDevice(PickFirstSuitablePhysicalDevice(VulkanAPI::GetPhysicalDevices(VulkInstance)));
		MainDevice.CreateLogicalDevice({}, {}, true);
	}

	RHICommandQueue* VulkanAPI::CreateCommandQueue(const ERHICommandQueueType type) const
	{
		VulkanCommandQueue* vulkanCmdQueue = new VulkanCommandQueue();
		vulkanCmdQueue->eType = type;
		vulkanCmdQueue->VkCommandQueue = MainDevice.RequestDeviceQueue(Conversion::ToVulkan(type));
		vulkanCmdQueue->VkCommandPool = VulkanAPI::CreateCommandPool(MainDevice.VkLogicalDevice, MainDevice.QueueFamilyIndices.Graphics);

		return vulkanCmdQueue;
	}

	RHISwapChain* VulkanAPI::CreateSwapChain(HINSTANCE i, HWND windowHandle, RHICommandQueue* commandQueue) const
	{
		VulkanSwapChain* vulkanSwpChn = new VulkanSwapChain();
		
		// Fill in RHISwapchain data
		RECT rect;
		if (GetWindowRect(windowHandle, &rect))
		{
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			vulkanSwpChn->Width = (float)width;
			vulkanSwpChn->Height = (float)height;
		}

		// Create the VkSurface
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = windowHandle;
		createInfo.hinstance = i;

		VkSurfaceKHR vkTemp{};
		vkCreateWin32SurfaceKHR(VulkInstance, &createInfo, nullptr, &vkTemp);
		vulkanSwpChn->VkSurface = vkTemp;

		// Create the VkSwapChain
		const SwapChainSupportDetails& swpChnSupportDetails = VulkanAPI::QuerySwapChainSupport(MainDevice.VkPhysicalDevice, vulkanSwpChn->VkSurface);
		
		// Choose surface format: (todo, arbitrary)
		vk::SurfaceFormatKHR chosenFormat{};
		for (const auto& availableFormat : swpChnSupportDetails.VkFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
			{
				chosenFormat = availableFormat;
				break;
			}
		}

		// Choose Present mode: (todo, arbitrary)
		vk::PresentModeKHR chosenPresentMode = vk::PresentModeKHR::eFifo; // default
		for (const auto& availablePresentMode : swpChnSupportDetails.VkPresentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) 
			{
				chosenPresentMode = availablePresentMode;
				break;
			}
		}
		
		// Setup Extent
		vk::Extent2D chosenSwapExtent{};
		const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = swpChnSupportDetails.VkSurfaceCapabilities;
		if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			chosenSwapExtent = surfaceCapabilities.currentExtent;
		}
		else
		{
			int w, h;
			vk::Extent2D actualExtent = { (uint32_t)vulkanSwpChn->Width, (uint32_t)vulkanSwpChn->Height };
			actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
			chosenSwapExtent = actualExtent;
		}
		
		vk::SwapchainCreateInfoKHR swpChnCreateInfo{};
		swpChnCreateInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
		swpChnCreateInfo.surface = vulkanSwpChn->VkSurface;
		swpChnCreateInfo.minImageCount = vulkanSwpChn->NumBackBuffers;
		swpChnCreateInfo.imageFormat = chosenFormat.format;
		swpChnCreateInfo.imageColorSpace = chosenFormat.colorSpace;
		swpChnCreateInfo.imageExtent = chosenSwapExtent;
		swpChnCreateInfo.imageArrayLayers = 1;
		swpChnCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		swpChnCreateInfo.presentMode = chosenPresentMode;
		swpChnCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swpChnCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swpChnCreateInfo.clipped = true;
		swpChnCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		vulkanSwpChn->VkSwapChain = MainDevice.VkLogicalDevice.createSwapchainKHR(swpChnCreateInfo, nullptr);

		// Get Vulkan swapchain images:
		vulkanSwpChn->VkSwapChainImages = MainDevice.VkLogicalDevice.getSwapchainImagesKHR(vulkanSwpChn->VkSwapChain);
		vulkanSwpChn->VkExtent = chosenSwapExtent;
		vulkanSwpChn->VkImageFormat = chosenFormat.format;

		// Create Image views:
		vulkanSwpChn->VkImageViews = std::vector<vk::ImageView>{};
		for (int i = 0; i < vulkanSwpChn->VkSwapChainImages.size(); ++i)
			vulkanSwpChn->VkImageViews.push_back(VulkanAPI::CreateImageView(MainDevice.VkLogicalDevice, vulkanSwpChn->VkSwapChainImages[i], chosenFormat.format));

		return vulkanSwpChn;
	}

	VulkanAPI::~VulkanAPI()
	{
		VulkInstance.destroyDebugUtilsMessengerEXT(debugMessageFunc);
		vkDestroyInstance(VulkInstance, nullptr);
	}

	const VulkanAPI::VulkanDevice& VulkanAPI::GetDevice()
	{
		return MainDevice;
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

	/* Statics */
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

		// EXTENSIONS
		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<vk::ExtensionProperties> extensions(extensionCount);
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		char** extensionNames = new char* [extensionCount] {};
		for (int i = 0; i < extensionCount; ++i)
		{
			extensionNames[i] = extensions[i].extensionName;
		}

		std::vector<const char*> vExtensions(extensionNames, extensionNames + extensionCount);

		

		std::vector<const char*> layers{};
#if _DEBUG
		// VALIDATION LAYERS
		if (validation)
		{
			// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
			// Note that on Android this layer requires at least NDK r20
			layers.push_back("VK_LAYER_KHRONOS_validation");

			// One of the requested validationLayers is not available!...
			assert(CheckValidationLayerSupport(layers));

			vExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
#endif
		
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(vExtensions.size());
		createInfo.ppEnabledExtensionNames = vExtensions.data();

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

		return true;
	}

	vk::CommandPool VulkanAPI::CreateCommandPool(const vk::Device& device, uint32_t queueFamilyIndex)
	{
		vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
		return device.createCommandPool(createInfo);
	}

	VulkanAPI::SwapChainSupportDetails VulkanAPI::QuerySwapChainSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
	{
		SwapChainSupportDetails details;

		details.VkSurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		details.VkFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		details.VkPresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

		return details;
	}

	vk::ImageView VulkanAPI::CreateImageView(const vk::Device& device, vk::Image image, vk::Format imageFormat, vk::ImageViewType viewType)
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eImageViewCreateInfo;
		createInfo.image = image;
		createInfo.viewType = viewType;
		createInfo.format = imageFormat; // No, we cannot query this :(
		createInfo.components = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity };
		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		return device.createImageView(createInfo, nullptr);
	}

	vk::CommandBuffer VulkanCommandList::GetVulkanCommandBuffer() const
	{
		return VkCommandBuffer;
	}

	/* VulkanCommandQueue */
	RHICommandList* VulkanCommandQueue::SetupNewCommandList(GraphicsAPI* api)
	{
		VulkanCommandList* newVulkanCommandList = new VulkanCommandList();
		VulkanAPI* vulkanApi = (VulkanAPI*)api;

		/* Allocate a new command list */
		newVulkanCommandList->VkCommandBuffer = 
			vulkanApi->GetDevice().VkLogicalDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo(VkCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();

		return newVulkanCommandList;
	}
	
	vk::Queue VulkanCommandQueue::GetVulkanQueue() const
	{
		return VkCommandQueue;
	}

	VulkanCommandQueue::~VulkanCommandQueue()
	{
		
	}
	
	/* Vulkan Device */
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

	vk::Queue VulkanAPI::VulkanDevice::RequestDeviceQueue(vk::QueueFlagBits queueType, uint32_t queueIndex) const
	{
		return VkLogicalDevice.getQueue(GetQueueFamilyIndex(queueType), queueIndex);
	}

	/* VulkanSwapchain */
	void VulkanSwapChain::Present(RHICommandQueue* commandQueue, bool VSync)
	{
		VulkanCommandQueue* vulkanCommandQueue = (VulkanCommandQueue*)commandQueue;

		vk::PresentInfoKHR presentInfo{};
		presentInfo.sType = vk::StructureType::ePresentInfoKHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &VkSwapChain;
		presentInfo.pImageIndices = &CurrentBackBufferIndex;
		
		vulkanCommandQueue->GetVulkanQueue().presentKHR(&presentInfo);
	}

	void VulkanSwapChain::Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY)
	{

	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		vkDestroySurfaceKHR(VulkanAPI::Get().GetInstance(), VkSurface, nullptr);
	}

}

