#include "rhi_pch.h"
#include "influx_rhi.h"

#if INFLUX_RHI_VULKAN
#include "vulkan.hpp"

namespace influx::rhi
{
	using vk_physdevice	= VkPhysicalDevice;
	using vk_queue		= VkQueue;
	using vk_device		= VkDevice;
	using vk_buffer		= VkBuffer;
	using vk_image		= VkImage;

	template <typename _t, typename _p>
	inline result<_t*> cast(_p* ptr)
	{
		if (ptr == nullptr)
			return result<_t*>::make_error("cannot cast when ptr is nullptr!");

		_t* res = (_t*)ptr;
		if (res) return res;
		else return result<_t*>::make_error("failed casting ptr to type!");
	}
	inline string vkres_to_string(const VkResult& vkres)
	{
		return "";
	}
	template <class _t = char>
	inline result<_t> vkres_to_result(const VkResult& vkres, const _t& value_if_success)
	{
		using result_type = result<_t>;
		if (vkres != VK_SUCCESS)
			return result_type::make_error(vkres_to_string(vkres).c_str());
		return value_if_success;
	}

	VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

	result<object_native> create_native(const device_create_args& args, device_data* out_data)
	{
		using result_type = result<object_native>;
		auto physdevice = cast<vk_physdevice>(args.m_physdevice);
		if (!physdevice) return result_type::make_error("failed casting args.m_physdevice to vk_physdevice");

		// make a VkInstance
		VkInstance instance{};
		{
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = args.m_app_name;
			app_info.applicationVersion = args.m_app_version;
			app_info.pEngineName = args.m_engine_name;
			app_info.engineVersion = args.m_engine_version;
			app_info.apiVersion = args.m_api_version;

			// instance extensions
			vector<const char*> extensions;
			if (args.m_debug)
			{
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}

			// Check for available extensions
#if 0
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
#endif

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &app_info;
			createInfo.enabledExtensionCount = (uint32_t)extensions.size();
			createInfo.ppEnabledExtensionNames = extensions.data();

			auto instance_create_res = vkres_to_result(
				vkCreateInstance(&createInfo, nullptr /*pAllocator*/, &instance), instance);
			if (!instance_create_res) return result_type::make_error("failed creating VkInstance");
		}

		// debug callback
		if (args.m_debug)
		{
			VkDebugReportCallbackCreateInfoEXT create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			create_info.pfnCallback = (PFN_vkDebugReportCallbackEXT)debug_callback;
			create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

			VkDebugReportCallbackEXT callback{};

			PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
			auto create_debugcallback = vkres_to_result(CreateDebugReportCallback(instance, &create_info, nullptr, &callback), 0u);
			if (!create_debugcallback) return result_type::make_error("failed creating debug callback!");
		}

		// if no physical device is specified,
		// we should query our own...
		VkPhysicalDevice vkphysdevice = {};
		device_create_args adjusted_args = args;
		if (args.m_physdevice == nullptr)
		{
			// first pass: count the num devices
			uint32_t num_devices_found = 0;
			VkResult vkres = vkEnumeratePhysicalDevices(instance, &num_devices_found, nullptr);
			if (num_devices_found == 0u)
				return result_type::make_error("no physical GPUs found!");

			// second pass, get the device info
			num_devices_found = 1;
			vkres = vkEnumeratePhysicalDevices(instance, &num_devices_found, &vkphysdevice);
			if (num_devices_found == 0u)
				return result_type::make_error("no physical GPUs (that suppport vulkan) found!");

			// Check device features
			// Note: will apiVersion >= appInfo.apiVersion? Probably yes, but spec is unclear.
			VkPhysicalDeviceProperties device_props;
			VkPhysicalDeviceFeatures device_features;
			vkGetPhysicalDeviceProperties(vkphysdevice, &device_props);
			vkGetPhysicalDeviceFeatures(vkphysdevice, &device_features);

			uint32_t supportedVersion[] = {
				VK_VERSION_MAJOR(device_props.apiVersion),
				VK_VERSION_MINOR(device_props.apiVersion),
				VK_VERSION_PATCH(device_props.apiVersion)
			};
			// std::cout << "physical device supports version " << supportedVersion[0] << "." << supportedVersion[1] << "." << supportedVersion[2] << std::endl;

			adjusted_args.m_physdevice = vkphysdevice;
		}

		// queue families
		uint32 num_queue_families = 0u;
		uint32 queue_family_graphics = 0u;
		uint32 queue_family_present = 0u;
		{
			// 1: query num queue families
			vkGetPhysicalDeviceQueueFamilyProperties(vkphysdevice, &num_queue_families, nullptr);
			if (num_queue_families == 0u)
				return result_type::make_error("failed finding any queue-families");

			// 2: find queue family with graphics support
			vector<VkQueueFamilyProperties> queue_fam_props(num_queue_families);
			vkGetPhysicalDeviceQueueFamilyProperties(vkphysdevice, &num_queue_families, queue_fam_props.data());

			bool found_graphics_queue_family = false;
			bool found_present_queue_family = false;
			for (uint32_t i = 0; i < num_queue_families; i++)
			{
				VkBool32 support_present = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(vkphysdevice, i, nullptr, &support_present);

				// prefer graphics queue WITH present support
				if (queue_fam_props[i].queueCount > 0 && queue_fam_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queue_family_graphics = i;
					found_graphics_queue_family = true;
					if (support_present)
					{
						queue_family_present = i;
						found_present_queue_family = true;
						break;
					}
				}

				// try any other queue that supports present
				if (!found_present_queue_family && support_present)
				{
					queue_family_present = i;
					found_present_queue_family = true;
				}
			}

			if (!found_graphics_queue_family) return result_type::make_error("GPU does not support graphics queue family!");
			if (!found_present_queue_family) return result_type::make_error("GPU does not support present queue family!");
		}

		// create queues
		VkDeviceQueueCreateInfo queue_create_infos[1] = {};
		{
			const float queue_prio = 1.0f;
			queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_infos[0].queueFamilyIndex = queue_family_graphics;
			queue_create_infos[0].queueCount = 1;
			queue_create_infos[0].pQueuePriorities = &queue_prio;

#if 0 // separate present queue is optional
			queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_infos[1].queueFamilyIndex = queue_family_present;
			queue_create_infos[1].queueCount = 1;
			queue_create_infos[1].pQueuePriorities = &queue_prio;
#endif
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		{
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pQueueCreateInfos = queue_create_infos;
			deviceCreateInfo.queueCreateInfoCount = 1;
		}
		
		// device-features
		VkPhysicalDeviceFeatures enabledFeatures = {};
		{
			enabledFeatures.shaderClipDistance = VK_TRUE;
			enabledFeatures.shaderCullDistance = VK_TRUE;
		}
		
		// device-extensions
		{
			const char* deviceExtensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			deviceCreateInfo.enabledExtensionCount = 1;
			deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions;
			deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		}

		// device-layers (validation layer)
		if (args.m_debug)
		{
			const char* k_debug_layer = "VK_LAYER_LUNARG_standard_validation";
			deviceCreateInfo.enabledLayerCount = 1;
			deviceCreateInfo.ppEnabledLayerNames = &k_debug_layer;
		}

		// create the device
		VkDevice vkdevice{};
		VkResult vkres = vkCreateDevice(*physdevice.get(), &deviceCreateInfo, nullptr, &vkdevice);

		VkQueue vkqueue_graphics{};
		VkQueue vkqueue_present{};
		vkGetDeviceQueue(vkdevice, queue_family_graphics, 0, &vkqueue_graphics);
		vkGetDeviceQueue(vkdevice, queue_family_present, 0, &vkqueue_present);

		// todo: query memory properties
		// vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties, &vkdevice);

		return vkdevice;
	}
	result<object_native> create_native(const queue_create_args& args, queue_data* out_data)
	{
		vk_queue vkqueue{};
		return vkqueue;
	}
	result<object_native> create_native(const swapchain_create_args& args, swapchain_data* out_data);
	result<object_native> create_native(const descheap_create_args& args, descheap_data* out_data);
	result<object_native> create_native(const commandallocator_create_args& args, commandallocator_data* out_data);
	result<object_native> create_native(const commandlist_create_args& args, commandlist_data* out_data);
	result<object_native> create_native(const fence_create_args& args, fence_data* out_data);
	result<object_native> create_native(const buffer_create_args& args, buffer_data* out_data);
	result<object_native> create_native(const texture2D_create_args& args, texture2D_data* out_data);
	result<object_native> create_native(const texture3D_create_args& args, texture3D_data* out_data);
	result<object_native> create_native(const pipeline_create_args& args, pipeline_data* out_data);
	result<object_native> create_native(const rootsignature_create_args& args, rootsignature_data* out_data);

	VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData) {
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) 
		{
			std::cerr << "ERROR: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
		}
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) 
		{
			std::cerr << "WARNING: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
		}
		return VK_FALSE;
	}
}
#endif // INFLUX_RHI_VULKAN