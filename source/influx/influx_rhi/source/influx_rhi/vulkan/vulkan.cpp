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

	class static_pixel_formats final
	{
		static constexpr uint32 k_num_vkformats = 185u;
		std::pair<const char*, pixelformat> m_formats[k_num_vkformats];
		bool m_initialized = false;

	public:
		static static_pixel_formats& get()
		{
			static static_pixel_formats singleton;
			singleton.initialize();
			return singleton;
		}

		static bool is_supported(const pixelformat& format)
		{
			static_pixel_formats& formats = get();
			for (uint32 i = 0u; i < k_num_vkformats; ++i)
			{
				if (formats.m_formats[i].second == format) 
					return true;
			}
			return false;
		}
		static vk_format translate(const pixelformat& format)
		{
			static_pixel_formats& formats = get();
			for (uint32 i = 0u; i < k_num_vkformats; ++i)
			{
				if (formats.m_formats[i].second == format)
					return static_cast<vk_format>(i);
			}
			return VkFormat::VK_FORMAT_UNDEFINED;
		}

		static constexpr uint32 get_num_supported_formats()
		{ return k_num_vkformats; }

		void initialize();
	};

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

	VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

	inline result<VkDeviceMemory> allocate(
		vk_physdevice phys, 
		vk_device device, 
		uint64 bytesize, 
		VkMemoryPropertyFlags properties,
		uint32 typefilter)
	{
		using result_type = result<VkDeviceMemory>;

		vk_device vkdevice = device;
		vk_physdevice vkphys = phys;

		uint32 type_index = 0u;
		
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(vkphys, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typefilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				type_index = i;
				break;
			}
		}

		VkMemoryAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = bytesize;
		info.memoryTypeIndex = type_index;

		VkDeviceMemory vkmemory;
		auto vkres = vkAllocateMemory(vkdevice, &info, nullptr, &vkmemory);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkAllocateMemory failed!");

		return vkmemory;
	}

	inline static VkImageUsageFlags translate(e_resource_state state)
	{
		VkImageUsageFlags flags{};
		// if (has_flag(state, e_resource_state::common));
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
		return static_pixel_formats::translate(format);
	}
	inline static bool is_format_supported(const pixelformat& format)
	{
		return static_pixel_formats::is_supported(format);
	}
	inline static vk_surface_format translate_surface_format(const pixelformat& format)
	{
		VkSurfaceFormatKHR result{};
		result.colorSpace = translate_colorspace(format);
		result.format = translate_format(format);
		return result;
	}
	inline static uint64 get_pixelformat_bytesize(const pixelformat& format)
	{
		return format.get_bytes_per_pixel();
	}
	inline static VkImageType translate(const e_texture_type type)
	{
		switch (type)
		{
		case e_texture_type::texture1D: return VkImageType::VK_IMAGE_TYPE_1D;
		case e_texture_type::texture2D: return VkImageType::VK_IMAGE_TYPE_2D;
		case e_texture_type::texture3D: return VkImageType::VK_IMAGE_TYPE_3D;
		case e_texture_type::cubemap:	return VkImageType::VK_IMAGE_TYPE_2D;
		}
		return {};
	}
	inline static VkImageUsageFlags translate_image(const e_resource_bindflags flags)
	{
		VkImageUsageFlags result{};
		if (has_flag(flags, e_resource_bindflags::rtv)) result |= VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (has_flag(flags, e_resource_bindflags::dsv)) result |= VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (has_flag(flags, e_resource_bindflags::copysrc)) result |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (has_flag(flags, e_resource_bindflags::copydst)) result |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		return result;
	}
	inline static VkBufferUsageFlags translate_buffer(const e_resource_bindflags flags)
	{
		VkBufferUsageFlags result{};
		if (has_flag(flags, e_resource_bindflags::constbuffer)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (has_flag(flags, e_resource_bindflags::vertexbuffer)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (has_flag(flags, e_resource_bindflags::indexbuffer)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (has_flag(flags, e_resource_bindflags::indirectbuffer)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (has_flag(flags, e_resource_bindflags::copysrc)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (has_flag(flags, e_resource_bindflags::copydst)) result |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		return result;
	}
	inline static VkAttachmentDescription translate(const output_merger::per_rendertarget& target)
	{
		VkAttachmentDescription res{};
		res.format = translate_format(target.m_format);
		res.samples = VK_SAMPLE_COUNT_1_BIT;
		res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear at start
		res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // store so we can present
		res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		res.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return res;
	}
	inline static VkAttachmentDescription translate(const output_merger::per_depthtarget& target)
	{
		VkAttachmentDescription res{};
		res.format = translate_format(target.m_format);
		res.samples = VK_SAMPLE_COUNT_1_BIT;
		res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear at start
		res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // store so we can present
		res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		res.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return res;
	}
	inline static VkAttachmentDescription translate(const color_attachment& attachment)
	{
		VkAttachmentDescription res{};
		res.format = translate_format(attachment.m_format);
		res.samples = VK_SAMPLE_COUNT_1_BIT;
		res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear at start
		res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // store so we can present
		res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		res.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return res;
	}
	inline static VkAttachmentDescription translate(const depth_attachment& attachment)
	{
		VkAttachmentDescription res;
		res.format = translate_format(attachment.m_format); // e.g. VK_FORMAT_D32_SFLOAT
		res.samples = VK_SAMPLE_COUNT_1_BIT;
		res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear at start
		res.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // we don’t need depth after rendering
		res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		res.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		return res;
	}
	inline static VkRenderPassCreateFlags translate(e_renderpass_flags flags)
	{
		VkRenderPassCreateFlags res{};
		//if (has_flag(flags, e_renderpass_flags::allow_uav_write)) res |= VkRenderPassCreateFlagBits::
		return res;
	}
	inline static VkViewport translate(const viewport& viewport)
	{
		return {};
	}
	inline static VkRect2D translate(const xrect& rect)
	{
		return {};
	}
	inline static VkImageViewType translate(e_texture_type type, uint32 arraysize)
	{
		if (arraysize > 1u)
		{
			switch (type)
			{
			case e_texture_type::texture1D: return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			case e_texture_type::texture2D: return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			case e_texture_type::cubemap: return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			}
		}
		else
		{
			switch (type)
			{
			case e_texture_type::texture1D: return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D;
			case e_texture_type::texture2D: return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
			case e_texture_type::texture3D: return VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
			case e_texture_type::cubemap: return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
			}
		}
		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
	inline static VkShaderStageFlagBits translate(e_graphics_shader_slots shader)
	{
		switch (shader)
		{
		case e_graphics_shader_slots::vs: return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		case e_graphics_shader_slots::ps: return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		case e_graphics_shader_slots::ds: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case e_graphics_shader_slots::gs: return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		case e_graphics_shader_slots::hs: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case e_graphics_shader_slots::as: return VK_SHADER_STAGE_CALLABLE_BIT_NV;
		case e_graphics_shader_slots::ms: return VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
		}
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
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
#if 0
		if (args.m_debug)
		{
			const char* k_debug_layer = "VK_LAYER_LUNARG_standard_validation";
			deviceCreateInfo.enabledLayerCount = 1;
			deviceCreateInfo.ppEnabledLayerNames = &k_debug_layer;
		}
#endif

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
				texture.m_create_args.m_arraysize = 1u;
				texture.m_create_args.m_format = args.m_format;
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
	result<native_buffer> create_native(const buffer_create_args& args, buffer_data* out_data)
	{
		using result_type = result<native_buffer>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");
		if (args.m_bytesize == 0u)
			return result_type::make_error("args.m_bytesize MUST be bigger than 0!");

		// 1. create the buffer
		VkBuffer vkbuffer;
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = args.m_bytesize;
			info.usage = translate_buffer(args.m_bindflags); // VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			auto vkres = vkCreateBuffer(args.m_device, &info, nullptr, &vkbuffer);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateBuffer failed!");
		}

		// 2. Allocate memory (not shown in full) and bind
		VkDeviceMemory vkmemory;
		{
			VkMemoryRequirements memReq;
			vkGetBufferMemoryRequirements(args.m_device, vkbuffer, &memReq);

			VkMemoryPropertyFlags mempropFlags{};
			if (has_flag(args.m_memoryheap.m_flags, e_memoryheap_flags::cpu_visible))
				mempropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			
			auto allocation = allocate(args.m_physdevice, args.m_device, memReq.size, mempropFlags, memReq.memoryTypeBits);
			if (!allocation)
				return result_type::make_error("failed allocating GPU memory for buffer resource!");

			vkmemory = allocation.get();
			auto vkres = vkBindBufferMemory(args.m_device, vkbuffer, vkmemory, 0);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkBindBufferMemory failed!");
		}

		// 3. create view
		VkBufferView vkview;
		{
			VkBufferViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			info.buffer = vkbuffer;
			info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			info.offset = 0u;
			info.range = args.m_bytesize;
			
			auto vkres = vkCreateBufferView(args.m_device, &info, nullptr, &vkview);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateBufferView failed!");
		}

		if (out_data)
		{
			out_data->m_gpu_memory_address = vkmemory;
			out_data->m_buffer_view.m_cpu_address = 0u;
			out_data->m_buffer_view.m_gpu_address = 0u;
			out_data->m_buffer_view.m_native_view = vkview;
		}

		return vkbuffer;
	}
	result<native_texture> create_native(const texture_create_args& args, texture_data* out_data)
	{
		using result_type = result<native_texture>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		if (is_format_supported(args.m_format) == false)
			return result_type::make_error("args.m_format is not supported!");

		// 1. create the VkImage
		VkImage vkimage;
		{
			VkImageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = translate(args.m_type);
			info.format = translate_format(args.m_format);
			info.extent = { args.m_dimensions.x, args.m_dimensions.y, args.m_dimensions.z };
			info.mipLevels = args.m_num_mips;
			info.arrayLayers = args.m_arraysize;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = translate_image(args.m_bindflags);
			auto vkres = vkCreateImage(args.m_device, &info, nullptr, &vkimage);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateImage failed!");
		}
		
		// 2. allocate VRAM & bind
		// Allocate memory (not shown in full) and bind
		VkDeviceMemory vkmemory;
		{
			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(args.m_device, vkimage, &memReq);
			VkMemoryPropertyFlags mempropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			auto allocation = allocate(args.m_physdevice, args.m_device, memReq.size, mempropFlags, memReq.memoryTypeBits);
			if (!allocation)
				return result_type::make_error("failed allocating GPU memory for texture resource!");

			vkmemory = allocation.get();
			auto vkres = vkBindImageMemory(args.m_device, vkimage, vkmemory, 0u);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkBindImageMemory failed!");
		}
		
		// 3. create the VkImageView
		VkImageView vkview;
		if (args.m_create_view)
		{
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = vkimage;
			viewInfo.viewType = translate(args.m_type, args.m_arraysize);
			viewInfo.format = translate_format(args.m_format);
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.levelCount = args.m_num_mips;
			viewInfo.subresourceRange.layerCount = args.m_arraysize;

			auto vkres = vkCreateImageView(args.m_device, &viewInfo, nullptr, &vkview);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateImageView failed!");
		}

		// 4. store the add. data
		if (out_data)
		{
			out_data->m_gpu_memory_address = vkmemory;
			out_data->m_current_state = args.m_init_state;
			out_data->m_previous_state = args.m_init_state;
			out_data->m_texture_view.m_cpu_address = 0u;
			out_data->m_texture_view.m_gpu_address = 0u;
			out_data->m_texture_view.m_native_view = vkview;
		}

		return vkimage;
	}
	
	result<native_pipeline> create_native_graphics_pipeline(
		native_device device,
		const graphics_pipeline_desc& desc, 
		const graphics_shaderslots& shaders,
		pipeline_data* out_data)
	{
		using result_type = result<native_pipeline>;

		const bool has_depth = desc.m_output_merger.m_depthtarget.m_depth_enable;
		const uint32 num_colour_targets = desc.m_output_merger.get_num_enabled_rendertargets();

		const output_merger& output_merger = desc.m_output_merger;

		// create an implicit renderpass
		VkRenderPass renderpass;
		{
			// translate the attachments
			vector<VkAttachmentDescription> attachments{};
			uint32 num_attachments = has_depth ? num_colour_targets + 1 : num_colour_targets;
			attachments.reserve(num_attachments);
			for (uint32 i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
			{
				if (output_merger.m_rendertargets[i].m_enabled)
					attachments.push_back(translate(output_merger.m_rendertargets[i]));
			}
			if (has_depth) attachments.push_back(translate(output_merger.m_depthtarget));

			// make 1 subpass
			vector<VkSubpassDescription> subpasses{};
			vector<VkAttachmentReference> color_refs{};
			VkAttachmentReference depth_ref = {};
			{
				VkSubpassDescription subpass{};
				// translate the attachment references
				color_refs.resize(num_colour_targets);
				for (uint32 i = 0u; i < num_colour_targets; ++i)
				{
					color_refs[i].attachment = i;
					color_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				depth_ref.attachment = num_colour_targets; // at depth index 
				depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = color_refs.data();
				subpass.pDepthStencilAttachment = has_depth ? &depth_ref : nullptr;
				subpasses.push_back(subpass);
			}

			// no dependencies (1 subpass)
			vector<VkSubpassDependency> dependencies{};
			VkRenderPassCreateInfo info{};
			info.attachmentCount = num_attachments;
			info.dependencyCount = static_cast<uint32>(dependencies.size());
			info.flags = {};
			info.pAttachments = attachments.data();
			info.pDependencies = dependencies.data();
			info.pSubpasses = subpasses.data();
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.subpassCount = static_cast<uint32>(subpasses.size());
			auto vkres = vkCreateRenderPass(device, &info, nullptr, &renderpass);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateRenderPass failed!");
		}

		// create the shader modules
		vector<VkPipelineShaderStageCreateInfo> vkshaderinfos{};
		vector<VkShaderModule> vkmodules{};
		{
			VkShaderModuleCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			for (uint32 i = 0u; i < shaders.count; ++i)
			{
				const bool is_shader_set = shaders.is_set(i);
				const bool is_shader_optional = shaders.is_optional(i);
				const e_graphics_shader_slots shaderslot = shaders.get_type_at_index(i);
				if (is_shader_set)
				{
					const auto& shader = shaders.get(i);
					const auto& shaderinfo = shaders.get_info(i);
					info.codeSize = static_cast<uint32>(shader.size());
					info.pCode = (uint32*)shader.data();

					vkmodules.push_back({});
					VkShaderModule& current_module = vkmodules.back();
					auto vkres = vkCreateShaderModule(device, &info, nullptr, &current_module);
					if (vkres != VK_SUCCESS)
						return result_type::make_error("vkCreateShaderModule failed!");

					// Set up shader stage info
					VkPipelineShaderStageCreateInfo stageInfo = {};
					stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					stageInfo.stage = translate(shaderslot);
					stageInfo.module = current_module;
					stageInfo.pName = shaderinfo.m_name.c_str();
					vkshaderinfos.push_back(stageInfo);
				}
				else if (!is_shader_optional)
					return result_type::make_error("args is missing non-optional shader!");
			}
		}

		// Describe vertex input
		struct vertex { math::float3 m_position; math::float4 m_colour; };
		vector<VkVertexInputBindingDescription> vertex_bindings{};
		{
			VkVertexInputBindingDescription info{};
			info.binding = 0;
			info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			info.stride = sizeof(vertex);
			vertex_bindings.push_back(info);
		}
		vector<VkVertexInputAttributeDescription> vertex_attributes{};
		{
			VkVertexInputAttributeDescription info{};
			info.binding = 0;
			info.format = VK_FORMAT_R32G32B32_SFLOAT;
			info.location = 0;
			info.offset = offsetof(vertex, m_position);
			vertex_attributes.push_back(info);

			info.binding = 0;
			info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			info.location = 1;
			info.offset = offsetof(vertex, m_colour);
			vertex_attributes.push_back(info);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(vertex_bindings.size());
		vertexInputCreateInfo.pVertexBindingDescriptions = vertex_bindings.data();
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertex_attributes.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = vertex_attributes.data();

		// Describe input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

		// Describe viewport and scissor
		VkViewport viewport = translate(desc.m_default_viewport);
		VkRect2D scissor = translate(desc.m_default_xrect);

		// Note: scissor test is always enabled (although dynamic scissor is possible)
		// Number of viewports must match number of scissors
		VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.pViewports = &viewport;
		viewportCreateInfo.scissorCount = 1;
		viewportCreateInfo.pScissors = &scissor;

		// Describe rasterization
		// Note: depth bias and using polygon modes other than fill require changes to logical device creation (device features)
		VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
		rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationCreateInfo.depthBiasClamp = 0.0f;
		rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizationCreateInfo.lineWidth = 1.0f;

		// Describe multisampling
		// Note: using multisampling also requires turning on device features
		VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
		multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleCreateInfo.minSampleShading = 1.0f;
		multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

		// Describing color blending
		// Note: all paramaters except blendEnable and colorWriteMask are irrelevant here
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		// Note: all attachments must have the same values unless a device feature is enabled
		VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
		colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendCreateInfo.attachmentCount = 1;
		colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendCreateInfo.blendConstants[0] = 0.0f;
		colorBlendCreateInfo.blendConstants[1] = 0.0f;
		colorBlendCreateInfo.blendConstants[2] = 0.0f;
		colorBlendCreateInfo.blendConstants[3] = 0.0f;

		// Describe pipeline layout
		// Note: this describes the mapping between memory and shader resources (descriptor sets)
		// This is for uniform buffers and samplers
		VkDescriptorSetLayout vkrootsignature{};
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo = {};
			descriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayoutCreateInfo.bindingCount = 1;
			descriptorLayoutCreateInfo.pBindings = &layoutBinding;

			auto vkres = vkCreateDescriptorSetLayout(device, &descriptorLayoutCreateInfo, nullptr, &vkrootsignature);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateDescriptorSetLayout failed!");
		}

		VkPipelineLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.setLayoutCount = 1;
		layoutCreateInfo.pSetLayouts = &vkrootsignature;

		VkPipelineLayout vklayout;
		auto vkres = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &vklayout);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkCreatePipelineLayout failed!");

		// Create the graphics pipeline
		VkPipeline vkpipeline{};
		{
			VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.stageCount = static_cast<uint32>(vkshaderinfos.size());
			pipelineCreateInfo.pStages = vkshaderinfos.data();
			pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
			pipelineCreateInfo.pViewportState = &viewportCreateInfo;
			pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
			pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
			pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
			pipelineCreateInfo.layout = vklayout;
			pipelineCreateInfo.renderPass = renderpass;
			pipelineCreateInfo.subpass = 0;
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineCreateInfo.basePipelineIndex = -1;

			vkres = vkCreateGraphicsPipelines(device, nullptr, 1u, &pipelineCreateInfo, nullptr, &vkpipeline);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateGraphicsPipelines failed!");
		}
		
		if (out_data)
		{

		}
		return vkpipeline;
	}

	result<native_pipeline>	create_native(const pipeline_create_args& args, pipeline_data* out_data)
	{
		using result_type = result<native_pipeline>;

		result_type result;
		switch (args.m_type)
		{
		case e_pipeline_type::graphics:
			result = create_native_graphics_pipeline(args.m_device, args.m_graphics, args.m_graphics_shaders, out_data);
			break;

		case e_pipeline_type::compute:
		case e_pipeline_type::raytracing:
			return result_type::make_error("todo: noimpl!");
		}
		return result;
	}
	result<native_rootsignature> create_native(const rootsignature_create_args& args, rootsignature_data* out_data);

	// [buffer]
	result<void*> buffer::map_begin(const map_args& args)
	{
		using result_type = result<void*>;

		void* result = nullptr;
		VkDeviceSize size = args.m_bytesize;
		VkDeviceSize offset = args.m_offset;
		VkMemoryMapFlags flags{};
		VkDeviceMemory memory = m_data.m_gpu_memory_address;
		auto vkres = vkMapMemory(m_native_device, memory, offset, size, flags, &result);
		if (vkres != VK_SUCCESS)
			return result_type::make_error("vkMapMemory failed!");

		return result;
	}
	result<> buffer::map_end()
	{
		VkDeviceMemory memory = m_data.m_gpu_memory_address;
		vkUnmapMemory(m_native_device, memory);
		return {};
	}
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
		for (commandlist*& list : commandlists)
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
	result<> commandlist::draw(const draw_args& args)
	{
		vkCmdDraw(m_native_object,
			args.m_num_vertices, args.m_num_instances,
			args.m_start_vertex, args.m_start_instance);
		return {};
	}
	result<> commandlist::draw_indexed(const draw_indexed_args& args)
	{
		vkCmdDrawIndexed(m_native_object, args.m_num_indices, args.m_num_instances,
			args.m_start_index, args.m_start_vertex, args.m_start_instance);
		return {};
	}
	result<> commandlist::dispatch(const math::uint3& group_nums)
	{
		vkCmdDispatch(m_native_object, group_nums.x, group_nums.y, group_nums.z);
		return {};
	}
	result<> commandlist::renderpass_begin(device& device, const renderpass_args& args)
	{
		using result_type = result<>;

		if (!device.is_valid())
			return result_type::make_error("device is not valid!");

		// translate the attachments
		const bool has_depth = args.m_depth_attachment.m_is_enabled;
		vector<VkAttachmentDescription> attachments{};
		uint32 num_colour_attachments = static_cast<uint32>(args.m_color_attachments.size());
		uint32 num_attachments = has_depth ? num_colour_attachments + 1 : num_colour_attachments;
		attachments.reserve(num_attachments);
		for (const auto& att : args.m_color_attachments)
		{
			attachments.push_back(translate(att));
		}
		if (has_depth) attachments.push_back(translate(args.m_depth_attachment));

		// make 1 subpass
		vector<VkSubpassDescription> subpasses{};
		vector<VkAttachmentReference> color_refs{};
		VkAttachmentReference depth_ref = {};
		{
			VkSubpassDescription subpass{};
			// translate the attachment references
			color_refs.resize(num_colour_attachments);
			for (uint32 i = 0u; i < num_colour_attachments; ++i)
			{
				color_refs[i].attachment = i;
				color_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			depth_ref.attachment = num_colour_attachments;
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = color_refs.data();
			subpass.pDepthStencilAttachment = has_depth ? &depth_ref : nullptr;
			subpasses.push_back(subpass);
		}

		// no dependencies (1 subpass)
		vector<VkSubpassDependency> dependencies{};
		{

		}

		// make the renderpass
		VkRenderPass renderpass;
		{
			VkRenderPassCreateInfo info{};
			info.attachmentCount = num_attachments;
			info.dependencyCount = static_cast<uint32>(dependencies.size());
			info.flags = translate(args.m_flags);
			info.pAttachments = attachments.data();
			info.pDependencies = dependencies.data();
			info.pSubpasses = subpasses.data();
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.subpassCount = static_cast<uint32>(subpasses.size());
			auto vkres = vkCreateRenderPass(device.m_native_object, &info, nullptr, &renderpass);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateRenderPass failed!");
		}

		// make the framebuffer
		VkFramebuffer framebuffer;
		{
			vector<VkImageView> attachment_views{}; attachment_views.reserve(num_attachments);
			for (const auto& att : args.m_color_attachments)
			{
				attachment_views.push_back((VkImageView)att.m_rtv_descriptor.m_native_view);
			}
			if (has_depth) attachment_views.push_back((VkImageView)args.m_depth_attachment.m_dsv_descriptor.m_native_view);
			
			VkFramebufferCreateFlags flags{};
			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.attachmentCount = static_cast<uint32>(attachments.size());
			info.flags = flags;
			info.width = args.m_width;
			info.height = args.m_height;
			info.layers = 1u;
			info.pAttachments = attachment_views.data();
			info.renderPass = renderpass;
			auto vkres = vkCreateFramebuffer(device.m_native_object, &info, nullptr, &framebuffer);
			if (vkres != VK_SUCCESS)
				return result_type::make_error("vkCreateFramebuffer failed!");
		}

		// begin the renderpass
		vector<VkClearValue> clear_values{};

		VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE;
		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.framebuffer = framebuffer;
		info.pClearValues = clear_values.data();
		info.clearValueCount = static_cast<uint32>(clear_values.size());
		info.renderArea.extent = { args.m_width, args.m_height };
		info.renderPass = renderpass;
		vkCmdBeginRenderPass(m_native_object, &info, subpass_contents);

		return{};
	}
	result<> commandlist::renderpass_end()
	{
		vkCmdEndRenderPass(m_native_object);
		return{};
	}

	// [texture]
	result<uint64> texture::calculate_bytesize() const
	{
		using result_type = result<uint64>;
		return get_format().get_bytes_per_pixel() * get_num_pixels();
	}
	result<uint64> texture::calculate_bytestride() const
	{
		return get_format().get_bytes_per_pixel();
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
	
	void static_pixel_formats::initialize()
	{
		if (m_initialized) return;
		m_initialized = true;

		using namespace format;
		m_formats[VK_FORMAT_UNDEFINED					  ] = {"VK_FORMAT_UNDEFINED					   ", {} };
		m_formats[VK_FORMAT_R4G4_UNORM_PACK8			  ] = {"VK_FORMAT_R4G4_UNORM_PACK8			   ", {} };
		m_formats[VK_FORMAT_R4G4B4A4_UNORM_PACK16		  ] = {"VK_FORMAT_R4G4B4A4_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_B4G4R4A4_UNORM_PACK16		  ] = {"VK_FORMAT_B4G4R4A4_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_R5G6B5_UNORM_PACK16			  ] = {"VK_FORMAT_R5G6B5_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_B5G6R5_UNORM_PACK16			  ] = {"VK_FORMAT_B5G6R5_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_R5G5B5A1_UNORM_PACK16		  ] = {"VK_FORMAT_R5G5B5A1_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_B5G5R5A1_UNORM_PACK16		  ] = {"VK_FORMAT_B5G5R5A1_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_A1R5G5B5_UNORM_PACK16		  ] = {"VK_FORMAT_A1R5G5B5_UNORM_PACK16		   ", {} };
		m_formats[VK_FORMAT_R8_UNORM					  ] = {"VK_FORMAT_R8_UNORM					   ", {} };
		m_formats[VK_FORMAT_R8_SNORM					  ] = {"VK_FORMAT_R8_SNORM					   ", {} };
		m_formats[VK_FORMAT_R8_USCALED					  ] = {"VK_FORMAT_R8_USCALED				   ", {} };
		m_formats[VK_FORMAT_R8_SSCALED					  ] = {"VK_FORMAT_R8_SSCALED				   ", {} };
		m_formats[VK_FORMAT_R8_UINT						  ] = {"VK_FORMAT_R8_UINT					   ", {} };
		m_formats[VK_FORMAT_R8_SINT						  ] = {"VK_FORMAT_R8_SINT					   ", {} };
		m_formats[VK_FORMAT_R8_SRGB						  ] = {"VK_FORMAT_R8_SRGB					   ", {} };
		m_formats[VK_FORMAT_R8G8_UNORM					  ] = {"VK_FORMAT_R8G8_UNORM				   ", {} };
		m_formats[VK_FORMAT_R8G8_SNORM					  ] = {"VK_FORMAT_R8G8_SNORM				   ", {} };
		m_formats[VK_FORMAT_R8G8_USCALED				  ] = {"VK_FORMAT_R8G8_USCALED				   ", {} };
		m_formats[VK_FORMAT_R8G8_SSCALED				  ] = {"VK_FORMAT_R8G8_SSCALED				   ", {} };
		m_formats[VK_FORMAT_R8G8_UINT					  ] = {"VK_FORMAT_R8G8_UINT					   ", {} };
		m_formats[VK_FORMAT_R8G8_SINT					  ] = {"VK_FORMAT_R8G8_SINT					   ", {} };
		m_formats[VK_FORMAT_R8G8_SRGB					  ] = {"VK_FORMAT_R8G8_SRGB					   ", {} };
		m_formats[VK_FORMAT_R8G8B8_UNORM				  ] = {"VK_FORMAT_R8G8B8_UNORM				   ", {} };
		m_formats[VK_FORMAT_R8G8B8_SNORM				  ] = {"VK_FORMAT_R8G8B8_SNORM				   ", {} };
		m_formats[VK_FORMAT_R8G8B8_USCALED				  ] = {"VK_FORMAT_R8G8B8_USCALED			   ", {} };
		m_formats[VK_FORMAT_R8G8B8_SSCALED				  ] = {"VK_FORMAT_R8G8B8_SSCALED			   ", {} };
		m_formats[VK_FORMAT_R8G8B8_UINT					  ] = {"VK_FORMAT_R8G8B8_UINT				   ", {} };
		m_formats[VK_FORMAT_R8G8B8_SINT					  ] = {"VK_FORMAT_R8G8B8_SINT				   ", {} };
		m_formats[VK_FORMAT_R8G8B8_SRGB					  ] = {"VK_FORMAT_R8G8B8_SRGB				   ", {} };
		m_formats[VK_FORMAT_B8G8R8_UNORM				  ] = {"VK_FORMAT_B8G8R8_UNORM				   ", {} };
		m_formats[VK_FORMAT_B8G8R8_SNORM				  ] = {"VK_FORMAT_B8G8R8_SNORM				   ", {} };
		m_formats[VK_FORMAT_B8G8R8_USCALED				  ] = {"VK_FORMAT_B8G8R8_USCALED			   ", {} };
		m_formats[VK_FORMAT_B8G8R8_SSCALED				  ] = {"VK_FORMAT_B8G8R8_SSCALED			   ", {} };
		m_formats[VK_FORMAT_B8G8R8_UINT					  ] = {"VK_FORMAT_B8G8R8_UINT				   ", {} };
		m_formats[VK_FORMAT_B8G8R8_SINT					  ] = {"VK_FORMAT_B8G8R8_SINT				   ", {} };
		m_formats[VK_FORMAT_B8G8R8_SRGB					  ] = {"VK_FORMAT_B8G8R8_SRGB				   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_UNORM				  ] = {"VK_FORMAT_R8G8B8A8_UNORM			   ", {e_format::unorm,	{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
		m_formats[VK_FORMAT_R8G8B8A8_SNORM				  ] = {"VK_FORMAT_R8G8B8A8_SNORM			   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_USCALED			  ] = {"VK_FORMAT_R8G8B8A8_USCALED			   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_SSCALED			  ] = {"VK_FORMAT_R8G8B8A8_SSCALED			   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_UINT				  ] = {"VK_FORMAT_R8G8B8A8_UINT				   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_SINT				  ] = {"VK_FORMAT_R8G8B8A8_SINT				   ", {} };
		m_formats[VK_FORMAT_R8G8B8A8_SRGB				  ] = {"VK_FORMAT_R8G8B8A8_SRGB				   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_UNORM				  ] = {"VK_FORMAT_B8G8R8A8_UNORM			   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_SNORM				  ] = {"VK_FORMAT_B8G8R8A8_SNORM			   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_USCALED			  ] = {"VK_FORMAT_B8G8R8A8_USCALED			   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_SSCALED			  ] = {"VK_FORMAT_B8G8R8A8_SSCALED			   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_UINT				  ] = {"VK_FORMAT_B8G8R8A8_UINT				   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_SINT				  ] = {"VK_FORMAT_B8G8R8A8_SINT				   ", {} };
		m_formats[VK_FORMAT_B8G8R8A8_SRGB				  ] = {"VK_FORMAT_B8G8R8A8_SRGB				   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_UNORM_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_UNORM_PACK32		   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_SNORM_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_SNORM_PACK32		   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_USCALED_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_USCALED_PACK32	   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_SSCALED_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_SSCALED_PACK32	   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_UINT_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_UINT_PACK32		   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_SINT_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_SINT_PACK32		   ", {} };
		m_formats[VK_FORMAT_A8B8G8R8_SRGB_PACK32		  ] = {"VK_FORMAT_A8B8G8R8_SRGB_PACK32		   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_UNORM_PACK32	  ] = {"VK_FORMAT_A2R10G10B10_UNORM_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_SNORM_PACK32	  ] = {"VK_FORMAT_A2R10G10B10_SNORM_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_USCALED_PACK32	  ] = {"VK_FORMAT_A2R10G10B10_USCALED_PACK32   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_SSCALED_PACK32	  ] = {"VK_FORMAT_A2R10G10B10_SSCALED_PACK32   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_UINT_PACK32		  ] = {"VK_FORMAT_A2R10G10B10_UINT_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2R10G10B10_SINT_PACK32		  ] = {"VK_FORMAT_A2R10G10B10_SINT_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_UNORM_PACK32	  ] = {"VK_FORMAT_A2B10G10R10_UNORM_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_SNORM_PACK32	  ] = {"VK_FORMAT_A2B10G10R10_SNORM_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_USCALED_PACK32	  ] = {"VK_FORMAT_A2B10G10R10_USCALED_PACK32   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_SSCALED_PACK32	  ] = {"VK_FORMAT_A2B10G10R10_SSCALED_PACK32   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_UINT_PACK32		  ] = {"VK_FORMAT_A2B10G10R10_UINT_PACK32	   ", {} };
		m_formats[VK_FORMAT_A2B10G10R10_SINT_PACK32		  ] = {"VK_FORMAT_A2B10G10R10_SINT_PACK32	   ", {} };
		m_formats[VK_FORMAT_R16_UNORM					  ] = {"VK_FORMAT_R16_UNORM					   ", {} };
		m_formats[VK_FORMAT_R16_SNORM					  ] = {"VK_FORMAT_R16_SNORM					   ", {} };
		m_formats[VK_FORMAT_R16_USCALED					  ] = {"VK_FORMAT_R16_USCALED				   ", {} };
		m_formats[VK_FORMAT_R16_SSCALED					  ] = {"VK_FORMAT_R16_SSCALED				   ", {} };
		m_formats[VK_FORMAT_R16_UINT					  ] = {"VK_FORMAT_R16_UINT					   ", {} };
		m_formats[VK_FORMAT_R16_SINT					  ] = {"VK_FORMAT_R16_SINT					   ", {} };
		m_formats[VK_FORMAT_R16_SFLOAT					  ] = {"VK_FORMAT_R16_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R16G16_UNORM				  ] = {"VK_FORMAT_R16G16_UNORM				   ", {} };
		m_formats[VK_FORMAT_R16G16_SNORM				  ] = {"VK_FORMAT_R16G16_SNORM				   ", {} };
		m_formats[VK_FORMAT_R16G16_USCALED				  ] = {"VK_FORMAT_R16G16_USCALED			   ", {} };
		m_formats[VK_FORMAT_R16G16_SSCALED				  ] = {"VK_FORMAT_R16G16_SSCALED			   ", {} };
		m_formats[VK_FORMAT_R16G16_UINT					  ] = {"VK_FORMAT_R16G16_UINT				   ", {} };
		m_formats[VK_FORMAT_R16G16_SINT					  ] = {"VK_FORMAT_R16G16_SINT				   ", {} };
		m_formats[VK_FORMAT_R16G16_SFLOAT				  ] = {"VK_FORMAT_R16G16_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R16G16B16_UNORM				  ] = {"VK_FORMAT_R16G16B16_UNORM			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_SNORM				  ] = {"VK_FORMAT_R16G16B16_SNORM			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_USCALED			  ] = {"VK_FORMAT_R16G16B16_USCALED			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_SSCALED			  ] = {"VK_FORMAT_R16G16B16_SSCALED			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_UINT				  ] = {"VK_FORMAT_R16G16B16_UINT			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_SINT				  ] = {"VK_FORMAT_R16G16B16_SINT			   ", {} };
		m_formats[VK_FORMAT_R16G16B16_SFLOAT			  ] = {"VK_FORMAT_R16G16B16_SFLOAT			   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_UNORM			  ] = {"VK_FORMAT_R16G16B16A16_UNORM		   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_SNORM			  ] = {"VK_FORMAT_R16G16B16A16_SNORM		   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_USCALED		  ] = {"VK_FORMAT_R16G16B16A16_USCALED		   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_SSCALED		  ] = {"VK_FORMAT_R16G16B16A16_SSCALED		   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_UINT			  ] = {"VK_FORMAT_R16G16B16A16_UINT			   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_SINT			  ] = {"VK_FORMAT_R16G16B16A16_SINT			   ", {} };
		m_formats[VK_FORMAT_R16G16B16A16_SFLOAT			  ] = {"VK_FORMAT_R16G16B16A16_SFLOAT		   ", {} };
		m_formats[VK_FORMAT_R32_UINT					  ] = {"VK_FORMAT_R32_UINT					   ", {} };
		m_formats[VK_FORMAT_R32_SINT					  ] = {"VK_FORMAT_R32_SINT					   ", {} };
		m_formats[VK_FORMAT_R32_SFLOAT					  ] = {"VK_FORMAT_R32_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R32G32_UINT					  ] = {"VK_FORMAT_R32G32_UINT				   ", {} };
		m_formats[VK_FORMAT_R32G32_SINT					  ] = {"VK_FORMAT_R32G32_SINT				   ", {} };
		m_formats[VK_FORMAT_R32G32_SFLOAT				  ] = {"VK_FORMAT_R32G32_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R32G32B32_UINT				  ] = {"VK_FORMAT_R32G32B32_UINT			   ", {} };
		m_formats[VK_FORMAT_R32G32B32_SINT				  ] = {"VK_FORMAT_R32G32B32_SINT			   ", {} };
		m_formats[VK_FORMAT_R32G32B32_SFLOAT			  ] = {"VK_FORMAT_R32G32B32_SFLOAT			   ", {} };
		m_formats[VK_FORMAT_R32G32B32A32_UINT			  ] = {"VK_FORMAT_R32G32B32A32_UINT			   ", {} };
		m_formats[VK_FORMAT_R32G32B32A32_SINT			  ] = {"VK_FORMAT_R32G32B32A32_SINT			   ", {} };
		m_formats[VK_FORMAT_R32G32B32A32_SFLOAT			  ] = {"VK_FORMAT_R32G32B32A32_SFLOAT		   ", {} };
		m_formats[VK_FORMAT_R64_UINT					  ] = {"VK_FORMAT_R64_UINT					   ", {} };
		m_formats[VK_FORMAT_R64_SINT					  ] = {"VK_FORMAT_R64_SINT					   ", {} };
		m_formats[VK_FORMAT_R64_SFLOAT					  ] = {"VK_FORMAT_R64_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R64G64_UINT					  ] = {"VK_FORMAT_R64G64_UINT				   ", {} };
		m_formats[VK_FORMAT_R64G64_SINT					  ] = {"VK_FORMAT_R64G64_SINT				   ", {} };
		m_formats[VK_FORMAT_R64G64_SFLOAT				  ] = {"VK_FORMAT_R64G64_SFLOAT				   ", {} };
		m_formats[VK_FORMAT_R64G64B64_UINT				  ] = {"VK_FORMAT_R64G64B64_UINT			   ", {} };
		m_formats[VK_FORMAT_R64G64B64_SINT				  ] = {"VK_FORMAT_R64G64B64_SINT			   ", {} };
		m_formats[VK_FORMAT_R64G64B64_SFLOAT			  ] = {"VK_FORMAT_R64G64B64_SFLOAT			   ", {} };
		m_formats[VK_FORMAT_R64G64B64A64_UINT			  ] = {"VK_FORMAT_R64G64B64A64_UINT			   ", {} };
		m_formats[VK_FORMAT_R64G64B64A64_SINT			  ] = {"VK_FORMAT_R64G64B64A64_SINT			   ", {} };
		m_formats[VK_FORMAT_R64G64B64A64_SFLOAT			  ] = {"VK_FORMAT_R64G64B64A64_SFLOAT		   ", {} };
		m_formats[VK_FORMAT_B10G11R11_UFLOAT_PACK32		  ] = {"VK_FORMAT_B10G11R11_UFLOAT_PACK32	   ", {} };
		m_formats[VK_FORMAT_E5B9G9R9_UFLOAT_PACK32		  ] = {"VK_FORMAT_E5B9G9R9_UFLOAT_PACK32	   ", {} };
		m_formats[VK_FORMAT_D16_UNORM					  ] = {"VK_FORMAT_D16_UNORM					   ", {} };
		m_formats[VK_FORMAT_X8_D24_UNORM_PACK32			  ] = {"VK_FORMAT_X8_D24_UNORM_PACK32		   ", {} };
		m_formats[VK_FORMAT_D32_SFLOAT					  ] = {"VK_FORMAT_D32_SFLOAT				   ", {e_format::sfloat, {_d,_32} }};
		m_formats[VK_FORMAT_S8_UINT						  ] = {"VK_FORMAT_S8_UINT					   ", {} };
		m_formats[VK_FORMAT_D16_UNORM_S8_UINT			  ] = {"VK_FORMAT_D16_UNORM_S8_UINT			   ", {} };
		m_formats[VK_FORMAT_D24_UNORM_S8_UINT			  ] = {"VK_FORMAT_D24_UNORM_S8_UINT			   ", {} };
		m_formats[VK_FORMAT_D32_SFLOAT_S8_UINT			  ] = {"VK_FORMAT_D32_SFLOAT_S8_UINT		   ", {} };
		m_formats[VK_FORMAT_BC1_RGB_UNORM_BLOCK			  ] = {"VK_FORMAT_BC1_RGB_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_BC1_RGB_SRGB_BLOCK			  ] = {"VK_FORMAT_BC1_RGB_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_BC1_RGBA_UNORM_BLOCK		  ] = {"VK_FORMAT_BC1_RGBA_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_BC1_RGBA_SRGB_BLOCK			  ] = {"VK_FORMAT_BC1_RGBA_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_BC2_UNORM_BLOCK				  ] = {"VK_FORMAT_BC2_UNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC2_SRGB_BLOCK				  ] = {"VK_FORMAT_BC2_SRGB_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC3_UNORM_BLOCK				  ] = {"VK_FORMAT_BC3_UNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC3_SRGB_BLOCK				  ] = {"VK_FORMAT_BC3_SRGB_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC4_UNORM_BLOCK				  ] = {"VK_FORMAT_BC4_UNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC4_SNORM_BLOCK				  ] = {"VK_FORMAT_BC4_SNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC5_UNORM_BLOCK				  ] = {"VK_FORMAT_BC5_UNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC5_SNORM_BLOCK				  ] = {"VK_FORMAT_BC5_SNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC6H_UFLOAT_BLOCK			  ] = {"VK_FORMAT_BC6H_UFLOAT_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC6H_SFLOAT_BLOCK			  ] = {"VK_FORMAT_BC6H_SFLOAT_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC7_UNORM_BLOCK				  ] = {"VK_FORMAT_BC7_UNORM_BLOCK			   ", {} };
		m_formats[VK_FORMAT_BC7_SRGB_BLOCK				  ] = {"VK_FORMAT_BC7_SRGB_BLOCK			   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK		  ] = {"VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK		  ] = {"VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK	  ] = {"VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK	  ] = {"VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK	  ] = {"VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK	  ] = {"VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK	   ", {} };
		m_formats[VK_FORMAT_EAC_R11_UNORM_BLOCK			  ] = {"VK_FORMAT_EAC_R11_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_EAC_R11_SNORM_BLOCK			  ] = {"VK_FORMAT_EAC_R11_SNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_EAC_R11G11_UNORM_BLOCK		  ] = {"VK_FORMAT_EAC_R11G11_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_EAC_R11G11_SNORM_BLOCK		  ] = {"VK_FORMAT_EAC_R11G11_SNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ASTC_4x4_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_4x4_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_4x4_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_4x4_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_5x4_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_5x4_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_5x4_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_5x4_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_5x5_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_5x5_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_5x5_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_5x5_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_6x5_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_6x5_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_6x5_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_6x5_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_6x6_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_6x6_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_6x6_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_6x6_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x5_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_8x5_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x5_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_8x5_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x6_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_8x6_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x6_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_8x6_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x8_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_8x8_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_8x8_SRGB_BLOCK			  ] = {"VK_FORMAT_ASTC_8x8_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x5_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_10x5_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x5_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_10x5_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x6_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_10x6_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x6_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_10x6_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x8_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_10x8_UNORM_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x8_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_10x8_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_10x10_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_10x10_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ASTC_10x10_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_10x10_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_12x10_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_12x10_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ASTC_12x10_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_12x10_SRGB_BLOCK		   ", {} };
		m_formats[VK_FORMAT_ASTC_12x12_UNORM_BLOCK		  ] = {"VK_FORMAT_ASTC_12x12_UNORM_BLOCK	   ", {} };
		m_formats[VK_FORMAT_ASTC_12x12_SRGB_BLOCK		  ] = {"VK_FORMAT_ASTC_12x12_SRGB_BLOCK		   ", {} };

#if 0
		m_formats[VK_FORMAT_G8B8G8R8_422_UNORM = 1000156000,
		m_formats[VK_FORMAT_B8G8R8G8_422_UNORM = 1000156001,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM = 1000156002,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_420_UNORM = 1000156003,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM = 1000156004,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_422_UNORM = 1000156005,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM = 1000156006,
		m_formats[VK_FORMAT_R10X6_UNORM_PACK16 = 1000156007,
		m_formats[VK_FORMAT_R10X6G10X6_UNORM_2PACK16 = 1000156008,
		m_formats[VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
		m_formats[VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
		m_formats[VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
		m_formats[VK_FORMAT_R12X4_UNORM_PACK16 = 1000156017,
		m_formats[VK_FORMAT_R12X4G12X4_UNORM_2PACK16 = 1000156018,
		m_formats[VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
		m_formats[VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
		m_formats[VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
		m_formats[VK_FORMAT_G16B16G16R16_422_UNORM = 1000156027,
		m_formats[VK_FORMAT_B16G16R16G16_422_UNORM = 1000156028,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM = 1000156029,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_420_UNORM = 1000156030,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM = 1000156031,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_422_UNORM = 1000156032,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM = 1000156033,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_444_UNORM = 1000330000,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_444_UNORM = 1000330003,
		m_formats[VK_FORMAT_A4R4G4B4_UNORM_PACK16 = 1000340000,
		m_formats[VK_FORMAT_A4B4G4R4_UNORM_PACK16 = 1000340001,
		m_formats[VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK = 1000066000,
		m_formats[VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK = 1000066001,
		m_formats[VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK = 1000066002,
		m_formats[VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK = 1000066003,
		m_formats[VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK = 1000066004,
		m_formats[VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK = 1000066005,
		m_formats[VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK = 1000066006,
		m_formats[VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK = 1000066007,
		m_formats[VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK = 1000066008,
		m_formats[VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK = 1000066009,
		m_formats[VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK = 1000066010,
		m_formats[VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK = 1000066011,
		m_formats[VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK = 1000066012,
		m_formats[VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK = 1000066013,
		m_formats[VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
		m_formats[VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
		m_formats[VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
		m_formats[VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
		m_formats[VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
		m_formats[VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
		m_formats[VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
		m_formats[VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
		m_formats[VK_FORMAT_R16G16_S10_5_NV = 1000464000,
		m_formats[VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR = 1000470000,
		m_formats[VK_FORMAT_A8_UNORM_KHR = 1000470001,
		m_formats[VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
		m_formats[VK_FORMAT_G8B8G8R8_422_UNORM_KHR = VK_FORMAT_G8B8G8R8_422_UNORM,
		m_formats[VK_FORMAT_B8G8R8G8_422_UNORM_KHR = VK_FORMAT_B8G8R8G8_422_UNORM,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
		m_formats[VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
		m_formats[VK_FORMAT_R10X6_UNORM_PACK16_KHR = VK_FORMAT_R10X6_UNORM_PACK16,
		m_formats[VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
		m_formats[VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
		m_formats[VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
		m_formats[VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
		m_formats[VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
		m_formats[VK_FORMAT_R12X4_UNORM_PACK16_KHR = VK_FORMAT_R12X4_UNORM_PACK16,
		m_formats[VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
		m_formats[VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
		m_formats[VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
		m_formats[VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
		m_formats[VK_FORMAT_G16B16G16R16_422_UNORM_KHR = VK_FORMAT_G16B16G16R16_422_UNORM,
		m_formats[VK_FORMAT_B16G16R16G16_422_UNORM_KHR = VK_FORMAT_B16G16R16G16_422_UNORM,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
		m_formats[VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
		m_formats[VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
		m_formats[VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
		m_formats[VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
		m_formats[VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
		m_formats[VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT = VK_FORMAT_A4R4G4B4_UNORM_PACK16,
		m_formats[VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT = VK_FORMAT_A4B4G4R4_UNORM_PACK16,
		m_formats[VK_FORMAT_MAX_ENUM = 0x7FFFFFFF
#endif
	}
}
#endif // INFLUX_RHI_VULKAN