#include "VulkanAPI.h"

namespace Influx::Graphics
{
	VulkanAPI::VulkanAPI()
	{
		CreateInstance(VkInstance, "None");
	}

	VulkanAPI::~VulkanAPI()
	{
		vkDestroyInstance(VkInstance, nullptr);
	}

	void VulkanAPI::CreateInstance(vk::Instance& outResult, const std::string& appName)
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
		const std::vector<const char*>& validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		if (!CheckValidationLayerSupport(validationLayers))
		{
			// One of the requested validationLayers is not available!...
			assert(false);
		}

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif
		
		vk::Result result = vk::createInstance(&createInfo, nullptr, &outResult);

		delete[] extensionNames;
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
}

