#include "rhi_pch.h"

#include "influx_rhi.h"

#if INFLUX_RHI_VULKAN
#if INFLUX_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR 0
#include "vulkan.hpp"
#endif // INFLUX_PLATFORM_WINDOWS

namespace influx::rhi
{
	using vk_instance	= VkInstance;
	using vk_physdevice	= VkPhysicalDevice;
	using vk_swapchain  = VkSwapchainKHR;
	using vk_queue		= VkQueue;
	using vk_device		= VkDevice;
	using vk_buffer		= VkBuffer;
	using vk_image		= VkImage;
	using vk_semaphore	= VkSemaphore;
	using vk_fence		= VkFence;
	using vk_format		= VkFormat;
	using vk_surface_format = VkSurfaceFormatKHR;
	using vk_colorspace = VkColorSpaceKHR;

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

	namespace vk_default_callbacks
	{
		inline static void* vk_allocate(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope scope)
		{

		}
		inline static void vk_free(void* user_data, void* memory)
		{

		}
		inline static void vk_intern_alloc_notification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
		{

		}
		inline static void vk_intern_free_notification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
		{

		}
		inline static void* vk_reallocate(void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
		{

		}
	}
	inline static VkAllocationCallbacks make_allocation_callbacks(void* user_data)
	{
		return VkAllocationCallbacks
		{
			.pUserData = user_data,
			.pfnAllocation = vk_default_callbacks::vk_allocate,
			.pfnReallocation = vk_default_callbacks::vk_reallocate,
			.pfnFree = vk_default_callbacks::vk_free,
			.pfnInternalAllocation = vk_default_callbacks::vk_intern_alloc_notification,
			.pfnInternalFree = vk_default_callbacks::vk_intern_free_notification
		};
	}

	void* pUserData;
	PFN_vkAllocationFunction                pfnAllocation;
	PFN_vkReallocationFunction              pfnReallocation;
	PFN_vkFreeFunction                      pfnFree;
	PFN_vkInternalAllocationNotification    pfnInternalAllocation;
	PFN_vkInternalFreeNotification          pfnInternalFree;

	VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

	inline static VkImageUsageFlags translate(e_resource_state state)
	{
		VkImageUsageFlags flags{};
		if (has_flag(state, e_resource_state::common)); // nothing
		if (has_flag(state, e_resource_state::copy_src)) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (has_flag(state, e_resource_state::copy_dst)) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		// ...
		return flags;
	}
	inline static VkExtent2D translate(const math::uint2& extent)
	{
		return VkExtent2D{ extent.x, extent.y };
	}
	inline static vk_colorspace translate_colorspace(const pixelformat& format)
	{
		vk_colorspace result{};
		return result;
	}
	inline static vk_format translate_format(const pixelformat& format)
	{
		vk_format result{};
		return result;
	}
	inline static vk_surface_format translate_surface_format(const pixelformat& format)
	{
		VkSurfaceFormatKHR result{};
		result.colorSpace = translate_colorspace(format);
		result.format = translate_format(format);
		return result;
	}

	inline result<VkSurfaceKHR> create_platform_window_surface(
		vk_instance instance,
		platform_window_handle window, 
		platform_instance_handle platform_instance)
	{
		using result_type = result<VkSurfaceKHR>;
		VkSurfaceKHR result{};
#if INFLUX_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hwnd = (HWND)window;
		info.hinstance = (HINSTANCE)platform_instance;
		auto vkresult = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &result);
		if (vkresult != VK_SUCCESS)
			return result_type::make_error("vkCreateWin32SurfaceKHR failed!");

		return result;
#else
		static_assert(false);
#endif
	}

	result<native_device> create_native(const device_create_args& args, device_data* out_data)
	{
		using result_type = result<native_device>;
	
		// make a VkInstance
		vk_instance instance{};
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
			const bool swapchain_extensions = true;
			if (swapchain_extensions)
			{
				extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if INFLUX_PLATFORM_WINDOWS
				extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
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
		vk_physdevice out_vkphysdevice = args.m_physdevice.has_value() ? args.m_physdevice.value()  : nullptr;
		if (out_vkphysdevice == nullptr)
		{
			// first pass: count the num devices
			uint32_t num_devices_found = 0;
			VkResult vkres = vkEnumeratePhysicalDevices(instance, &num_devices_found, nullptr);
			if (num_devices_found == 0u)
				return result_type::make_error("no physical GPUs found!");

			// second pass, get the device info
			vector<vk_physdevice> found_devices{}; found_devices.resize(num_devices_found);
			vkres = vkEnumeratePhysicalDevices(instance, &num_devices_found, found_devices.data());
			if (num_devices_found == 0u)
				return result_type::make_error("no physical GPUs (that suppport vulkan) found!");

			for (uint32 i = 0u; i < num_devices_found; ++i)
			{
				// Check device features
				// Note: will apiVersion >= appInfo.apiVersion? Probably yes, but spec is unclear.
				VkPhysicalDeviceProperties device_props;
				VkPhysicalDeviceFeatures device_features;
				vkGetPhysicalDeviceProperties(found_devices[i], &device_props);
				vkGetPhysicalDeviceFeatures(found_devices[i], &device_features);

				uint32_t supportedVersion[] = {
					VK_VERSION_MAJOR(device_props.apiVersion),
					VK_VERSION_MINOR(device_props.apiVersion),
					VK_VERSION_PATCH(device_props.apiVersion)
				};
				// std::cout << "physical device supports version " << supportedVersion[0] << "." << supportedVersion[1] << "." << supportedVersion[2] << std::endl;
			}
			out_vkphysdevice = found_devices[0];
		}

		// physical device: query queue families
		uint32 num_queue_families = 0u;
		uint32 queue_family_graphics = 0u;
		uint32 queue_family_present = 0u;
		uint32 queue_family_compute = 0u;
		uint32 queue_family_transfer = 0u;
		{
			// 1: query num queue families
			vkGetPhysicalDeviceQueueFamilyProperties(out_vkphysdevice, &num_queue_families, nullptr);
			if (num_queue_families == 0u)
				return result_type::make_error("failed finding any queue-families");

			// 2: find queue family with graphics support
			vector<VkQueueFamilyProperties> queue_fam_props(num_queue_families);
			vkGetPhysicalDeviceQueueFamilyProperties(out_vkphysdevice, &num_queue_families, queue_fam_props.data());

			bool found_graphics_queue_family = false;
			bool found_present_queue_family = false;
			bool found_compute_queue_family = false;
			for (uint32_t i = 0; i < num_queue_families; i++)
			{
#if 0
				VkBool32 support_present = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(out_vkphysdevice, i, nullptr, &support_present);
#endif

				// prefer graphics queue WITH present support
				if (queue_fam_props[i].queueCount > 0 && queue_fam_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queue_family_graphics = i;
					found_graphics_queue_family = true;
					// if (support_present) // we're assuming here...
					{
						queue_family_present = i;
						found_present_queue_family = true;
						break;
					}
				}
				if (queue_fam_props[i].queueCount > 0u && queue_fam_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					queue_family_compute = i;
					found_compute_queue_family = true;
				}
				if (queue_fam_props[i].queueCount > 0u && queue_fam_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					queue_family_transfer = i;
				}

				// try any other queue that supports present
				if (!found_present_queue_family /* && support_present*/)
				{
					queue_family_present = i;
					found_present_queue_family = true;
				}
			}

			if (!found_graphics_queue_family)
				return result_type::make_error("GPU does not support graphics queue family!");
			if (!found_present_queue_family)
				return result_type::make_error("GPU does not support present queue family!");
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
		VkResult vkres = vkCreateDevice(out_vkphysdevice, &deviceCreateInfo, nullptr, &vkdevice);

		// todo: query memory properties
		// vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties, &vkdevice);

		if (out_data)
		{
			out_data->m_children.clear();
			out_data->m_descriptor_strides;
			out_data->m_instance = instance;
			out_data->m_physical_device = out_vkphysdevice;
			out_data->m_queue_families.set_index(e_queue_type::compute, queue_family_compute);
			out_data->m_queue_families.set_index(e_queue_type::graphics, queue_family_graphics);
			out_data->m_queue_families.set_index(e_queue_type::copy, queue_family_transfer);
			out_data->m_queue_families.set_index(e_queue_type::present, queue_family_present);
		}
		return vkdevice;
	}
	result<native_queue> create_native(const queue_create_args& args, queue_data* out_data)
	{
		using result_type = result<native_queue>;
		
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		if (!args.m_queue_families.is_set(args.m_type))
			return result_type::make_error("args.m_queue_families has no args.m_type index set!");

		VkDevice device = args.m_device;
		VkQueue queue{};
		uint32 queue_family_index = args.m_queue_families.get_index(args.m_type);
		vkGetDeviceQueue(device, queue_family_index, 0u, &queue);

		if (out_data)
		{
			
		}
		return queue;
	}
	result<native_swapchain> create_native(const swapchain_create_args& args, swapchain_data* out_data)
	{
		using result_type = result<native_swapchain>;

		if (args.m_instance == nullptr)
			return result_type::make_error("args.m_instance is nullptr!");
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");
		if (args.m_window == nullptr)
			return result_type::make_error("args.m_window is nullptr!");

		const bool is_present_queue_valid = args.m_queue_families.is_set(e_queue_type::present);
		const bool is_graphics_queue_valid = args.m_queue_families.is_set(e_queue_type::graphics);
		if (!is_present_queue_valid || !is_graphics_queue_valid)
			return result_type::make_error("creating a swapchain requires args.m_queue_families - graphics & present BOTH be valid!");

		VkSurfaceKHR surface{};
		{
			auto create_surface = create_platform_window_surface(
				args.m_instance, 
				args.m_window, 
				args.m_platform_instance);

			if (!create_surface)
				return result_type::make_error("create_platform_window_surface failed!");
			surface = create_surface.get();
		}

		// handle queue concurrency
		// if present == graphics, the images are not shared across queues.
		// else, we need to specify what queues can access the resource simultaneously
		VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		uint32 concurrent_queue_family_index_count = 0u;
		uint32* concurrent_queue_family_indices = nullptr;
		const uint32 present_queue_family = args.m_queue_families.get_index(e_queue_type::present);
		const uint32 graphics_queue_family = args.m_queue_families.get_index(e_queue_type::graphics);
		uint32 concurrent_queue_families[] = { graphics_queue_family, present_queue_family };
		if (present_queue_family != graphics_queue_family)
		{
			sharing_mode = VK_SHARING_MODE_CONCURRENT;
			concurrent_queue_family_index_count = 2u;
			concurrent_queue_family_indices = concurrent_queue_families;
		}

		const e_resource_state init_state = e_resource_state::copy_dst;
		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.flags = {};
		info.surface = surface;
		info.minImageCount = args.m_num_buffers;
		info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;//translate_format(args.m_format);
		info.imageColorSpace = translate_colorspace(args.m_format);
		info.imageExtent = translate(args.m_dimensions);
		info.imageArrayLayers = 1u;
		info.imageUsage = translate(init_state); // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.imageSharingMode = sharing_mode;
		info.queueFamilyIndexCount = concurrent_queue_family_index_count;
		info.pQueueFamilyIndices = concurrent_queue_family_indices;
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		info.clipped = VK_TRUE;
		info.oldSwapchain = nullptr;

		VkSwapchainKHR swapchain{};
		auto vkres = vkCreateSwapchainKHR(args.m_device, &info, nullptr, &swapchain);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkCreateSwapchainKHR failed!");

		if (out_data != nullptr)
		{
			vk_device vkdevice = args.m_device;

			// fetch the image count (just to be sure)
			uint32_t imageCount = 0;
			auto vkres = vkGetSwapchainImagesKHR(vkdevice, swapchain, &imageCount, nullptr);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkGetSwapchainImagesKHR failed!");

			// fetch the backbuffer images
			vector<VkImage> swapchainImages(imageCount);
			vkres = vkGetSwapchainImagesKHR(vkdevice, swapchain, &imageCount, swapchainImages.data());
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkGetSwapchainImagesKHR failed!");

			// store the backbuffer images as wrapped textures
			out_data->m_swapchain_textures.resize(args.m_num_buffers);
			for (uint32 i = 0u; i < args.m_num_buffers; ++i)
			{
				texture& texture = out_data->m_swapchain_textures[i];
				texture.m_create_args.m_init_state = init_state;
				texture.m_create_args.m_allow_uav = false;
				texture.m_create_args.m_arraysize = 1u;
				texture.m_data.m_format = args.m_format;
				texture.m_data.m_current_state = texture.m_data.m_previous_state = init_state;
				texture.m_native_device = vkdevice;
				texture.m_native_object = swapchainImages[i];
			}
		}
		return swapchain;
	}
	result<native_descheap> create_native(const descheap_create_args& args, descheap_data* out_data);
	result<native_commandpool> create_native(const commandpool_create_args& args, commandpool_data* out_data)
	{
		using result_type = result<native_commandpool>;
		
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		// translate commandlist type to queue type
		e_queue_type queue_type{};
		switch (args.m_type)
		{
		case e_commandlist_type::graphics: queue_type = e_queue_type::graphics;
		}

		if (args.m_queue_families.is_set(queue_type) == false)
			return result_type::make_error("args.m_queue_families has no index set for args.m_type");

		VkCommandPool pool{};
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = args.m_queue_families.get_index(queue_type);
		
		auto vkres = vkCreateCommandPool(args.m_device, &info, nullptr, &pool);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkCreateCommandPool failed!");

		if (out_data != nullptr)
		{
			out_data;
		}
		return pool;
	}
	result<native_commandlist> create_native(const commandlist_create_args& args, commandlist_data* out_data)
	{
		using result_type = result<native_commandlist>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		VkCommandPool commandpool = args.m_pool.has_value() ? args.m_pool.value() : nullptr;
		if (commandpool == nullptr)
		{
			commandpool_create_args pool_create_args{};
			pool_create_args.m_device = args.m_device;
			pool_create_args.m_queue_families = args.m_queue_families;
			pool_create_args.m_type = args.m_type;
			auto create_commandpool = create_native(pool_create_args);
			if (!create_commandpool)
				return result_type::make_error("create_native(commandpool) failed!");
			commandpool = create_commandpool.get();
		}

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandBufferCount = 1u;
		alloc_info.commandPool = commandpool;
		alloc_info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer commandbuffer = nullptr;
		auto vkres = vkAllocateCommandBuffers(args.m_device, &alloc_info, &commandbuffer);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkAllocateCommandBuffers failed!");

		VkFence fence = nullptr;
		VkSemaphore semaphore = nullptr;
		if (args.m_own_fence)
		{
#if 0 // this is for reference what dx12 would do...
			fence_create_args fence_args = fence_create_args::fence();
			fence_args.m_device = args.m_device;
			fence_args.m_init_value = 0u;
			fence_data fence_data{};
			auto create_fence = create_native(fence_args, &fence_data);
			if (!create_fence)
				return result_type::make_error("create_native(fence) failed!");
			fence = create_fence.get();
#endif
			semaphore_create_args sem_args{};
			sem_args.m_device = args.m_device;
			semaphore_data sem_data{};
			auto create_sem = create_native(sem_args, &sem_data);
			if (!create_sem)
				return result_type::make_error("create_native(semaphore) failed!");
			semaphore = create_sem.get();
		}

		if (out_data)
		{
			out_data->m_current_pool = commandpool;
			out_data->m_fence = fence;
			out_data->m_semaphore = semaphore;
			out_data->m_complete_value = 0u;
			out_data->m_state = e_commandlist_state::init;
		}
		return commandbuffer;
	}
	result<native_fence> create_native(const fence_create_args& args, fence_data* out_data)
	{
		using result_type = result<native_fence>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		VkFenceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags; // VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
		VkFence fence{};
		auto vkres = vkCreateFence(args.m_device, &info, nullptr, &fence);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkCreatefence failed!");

		if (out_data != nullptr)
		{

		}
		return fence;
	}
	result<native_semaphore> create_native(const semaphore_create_args& args, semaphore_data* out_data)
	{
		using result_type = result<native_semaphore>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		VkSemaphoreTypeCreateInfo timelineInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};
		VkSemaphoreCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineInfo,
		};

		VkSemaphore semaphore;
		auto vkres = vkCreateSemaphore(args.m_device, &info, nullptr, &semaphore);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkCreateSemaphore failed!");
		
		if (out_data != nullptr)
		{

		}
		return semaphore;
	}
	result<native_buffer> create_native(const buffer_create_args& args, buffer_data* out_data);
	result<native_texture> create_native(const texture_create_args& args, texture_data* out_data);
	result<native_pipeline>	create_native(const pipeline_create_args& args, pipeline_data* out_data)
	{
		using result_type = result<native_pipeline>;
		return {};
	}
	result<native_rootsignature> create_native(const rootsignature_create_args& args, rootsignature_data* out_data);

	// [fence]
	result<> fence::queue_signal(uint64 signal_value, const queue& queue)
	{
		using result_type = result<>;
		return {};
	}
	result<> fence::signal(uint64 value)
	{
		using result_type = result<>;
		return {};
	}
	result<> fence::wait_for_value(uint64 value)
	{
		using result_type = result<>;
		return {};
	}
	result<uint64> fence::query_value() const
	{
		using result_type = result<uint64>;
		return {};
	}

	// [queue]
	result<> queue::submit(vector<commandlist*> commandlists) const
	{
		using result_type = result<>;

		if (commandlists.empty())
			return result_type::make_warning({}, "commandlists.size() == 0, not an error, nothing happened...");

		vector<VkCommandBufferSubmitInfo>	vkbuffer_submit_infos{};
		vector<VkSemaphoreSubmitInfo>		vksemaphore_submit_infos{};
		for (commandlist* list : commandlists)
		{
			if (list->is_recording())
			{
				auto end_result = list->end();
				if (!end_result)
					return result_type::make_error("commandlist::end() failed!");
			}

			VkCommandBuffer vkbuffer = list->m_native_object;
			vkbuffer_submit_infos.push_back(VkCommandBufferSubmitInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.commandBuffer = vkbuffer,
				.deviceMask = 0,
			});

			vk_semaphore vksemaphore = list->m_data.m_semaphore;
			if (vksemaphore != nullptr)
			{
				vksemaphore_submit_infos.push_back(VkSemaphoreSubmitInfo{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = vksemaphore,
					.value = list->m_data.m_complete_value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, // signals after execution
					.deviceIndex = 0,
				});
			}
		}

		// don't specify semaphores to wait for... (for now)
		vector<VkSemaphore> wait_semaphores{};
		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo2 submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 0,
			.pWaitSemaphoreInfos = NULL,
			.commandBufferInfoCount = static_cast<uint32>(vkbuffer_submit_infos.size()),
			.pCommandBufferInfos = vkbuffer_submit_infos.data(),
			.signalSemaphoreInfoCount = static_cast<uint32>(vksemaphore_submit_infos.size()),
			.pSignalSemaphoreInfos = vksemaphore_submit_infos.data(),
		};

		vk_queue vkqueue = m_native_object;
		auto vkres = vkQueueSubmit2(vkqueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkQueueSubmit failed!");

		for (commandlist* list : commandlists)
		{
			list->m_data.m_state = e_commandlist_state::inflight;
		}

		return {};
	}

	// [commandlist]
	result<> commandlist::start(device& device)
	{
		using result_type = result<>;
		if (device.m_native_object == nullptr)
			return result_type::make_error("device.m_native_object is nullptr!");
		
		if (m_data.m_current_pool == nullptr)
		{
			commandpool_create_args args{};
			args.m_device = device.m_native_object;
			args.m_type = m_create_args.m_type;
			args.m_queue_families = device.m_data.m_queue_families;
			auto new_pool = create_native(args);
			if (!new_pool)
				return result_type::make_error("create_native(commandpool) failed!");
			m_data.m_current_pool = new_pool.get();
		}
		
		return start(m_data.m_current_pool);
	}
	result<> commandlist::start(native_commandpool pool)
	{
		using result_type = result<>;
		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");

#if 0 // don't need to reset the entire pool
		if (pool == nullptr)
			return result_type::make_error("pool is nullptr!");
		VkCommandPoolResetFlags flags{};
		auto vkres = vkResetCommandPool(device, pool, flags);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkResetCommandPool failed!");
#endif

		VkCommandBufferResetFlags flags{};
		auto vkres = vkResetCommandBuffer(m_native_object, flags);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkResetCommandBuffer failed!");

		VkCommandBufferBeginInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = 0,
			.pInheritanceInfo = nullptr // only for secondary buffers
		};
		vkres = vkBeginCommandBuffer(m_native_object, &info);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkBeginCommandBuffer failed!");

		m_data.m_complete_value += 1u;
		m_data.m_state = e_commandlist_state::recording;
		return {};
	}
	result<> commandlist::end()
	{
		using result_type = result<>;
		auto vkres = vkEndCommandBuffer(m_native_object);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkEndCommandBuffer failed!");

		m_data.m_state = e_commandlist_state::closed;
		return {};
	}
	result<> commandlist::submit(queue& queue)
	{
		using result_type = result<>;
		return queue.submit({ this });
	}
	result<> commandlist::wait_for_finish() const
	{
		using result_type = result<>;
		if (m_data.m_semaphore == nullptr)
			return {};

		vk_device vkdevice = m_native_device;
		if (vkdevice == nullptr)
			return result_type::make_error("m_native_device is nullptr!");

		// less-blocking check
		uint64 current_value = m_data.m_complete_value - 1u;
		while (current_value != m_data.m_complete_value)
		{
			auto vkres = vkGetSemaphoreCounterValue(vkdevice, m_data.m_semaphore, &current_value);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkGetSemaphoreCounterValue failed!");
		}

#if 0 // full blocking check
		uint64 value_to_wait = m_data.m_complete_value;
		VkSemaphoreWaitInfo info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_data.m_semaphore,
			.pValues = &value_to_wait,
		};

		auto vkresult = vkWaitSemaphores(vkdevice, &info, UINT64_MAX);
		if (vkresult != VK_SUCCESS)
			return result_type::make_error("vkWaitSemaphores failed!");
#endif 
		return {};
	}
	result<> commandlist::clear_rtv(descriptor rtv, const clear& clear)
	{
		using result_type = result<>;
		return result_type::make_error("NOOP");
	}
	result<> commandlist::clear_texture(device& device, const texture& texture, const clear& clear)
	{
		using result_type = result<>;
		if (texture.m_native_object == nullptr)
			return result_type::make_error("texture.m_native_object is nullptr!");

		const math::float4 colour = clear.m_colour;
		VkClearColorValue clear_value = {
			.float32 = {colour.r,colour.g,colour.b,colour.a}, // RGBA
		};

		vector<VkImageSubresourceRange> ranges = { {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		} };

		VkImage image = texture.m_native_object;
		vkCmdClearColorImage(
			m_native_object,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clear_value,
			static_cast<uint32>(ranges.size()),
			ranges.data());
		return {};
	}

	// [swapchain]
	result<> swapchain::acquire_backbuffer(native_device device)
	{
		using result_type = result<>;
		if (device == nullptr)
			return result_type::make_error("device is nullptr!");
		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");

		m_data.m_backbuffer_info.m_is_acquired = false;

		uint32_t imageIndex;
		auto vkres = vkAcquireNextImageKHR(device, m_native_object, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkAcquireNextImageKHR failed!");

		m_data.m_backbuffer_info.m_is_acquired = true;
		m_data.m_backbuffer_info.m_current_index = imageIndex;
		return {};
	}
	result<> swapchain::present(const present_args& args) const
	{
		using result_type = result<>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");
		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");
		if (args.m_present_queue == nullptr)
			return result_type::make_error("args.m_present_queue is nullptr!");
		const bool backbuffer_acquired = m_data.m_backbuffer_info.m_is_acquired;
		const uint32 backbuffer_index = m_data.m_backbuffer_info.m_current_index;
		if (!backbuffer_acquired)
			return result_type::make_error("m_data.m_backbuffer_info.m_is_acquired MUST be true (call swapchain::acquire_backbuffer)");

		vk_device vkdevice = args.m_device;
		vk_swapchain vkswapchain = m_native_object;
		vk_queue vkqueue = args.m_present_queue;

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0u; // specify no wait!
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vkswapchain;
		presentInfo.pImageIndices = &backbuffer_index;
		presentInfo.pResults = nullptr; // Optional
		auto vkres = vkQueuePresentKHR(vkqueue, &presentInfo);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkQueuePresentKHR failed!");
		
		return {};
	}
	result<texture>	swapchain::get_backbuffer_resource(uint32 index) const
	{
		using result_type = result<texture>;
		if (index >= m_data.m_swapchain_textures.size())
			return result_type::make_error("index >= num_resources");
		return m_data.m_swapchain_textures[index];
	}
	result<texture>	swapchain::get_backbuffer_resource() const
	{
		using result_type = result<texture>;
		if (m_data.m_backbuffer_info.m_is_acquired == false)
			return result_type::make_error("backbuffer not yet acquired! call acquire_backbuffer() first!");

		const uint32 current_index = m_data.m_backbuffer_info.m_current_index;
		return m_data.m_swapchain_textures[current_index];
	}
	
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