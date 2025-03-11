#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_device.h"
#include "vk_headers.h"

// influx::core
#include "core/log.h"

// influx::graphics::vulkan
#include "influx_graphics/vulkan/vk_helpers.h"
#include "influx_graphics/vulkan/vk_conversion.h"
#include "influx_graphics/vulkan/vk_queue.h"
#include "influx_graphics/vulkan/vk_fence.h"
#include "influx_graphics/vulkan/vk_swapchain.h"
#include "influx_graphics/vulkan/vk_resource.h"
#include "influx_graphics/vulkan/vk_commandlist.h"
#include "influx_graphics/vulkan/vk_allocator.h"
#include "influx_graphics/vulkan/vk_resource_views.h"
#include "influx_graphics/vulkan/vk_descriptorheap.h"

namespace influx::graphics
{
	inline void on_debug_message(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT& data)
	{
		const bool log_warning	= severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		const bool log_error	= severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		e_log_category category{};
		if (log_error)
		{
			category = e_log_category::error;
		}
		else if (log_warning)
		{
			category = e_log_category::warning;
		}
		else
		{
			category = e_log_category::normal;
		}

		log(category, "vk_error: {} {}: {}", 
			data.messageIdNumber, data.pMessageIdName, data.pMessage);

#if 0
		std::cerr << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity)) << ": "
			<< vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type)) << ":\n";
		std::cerr << std::string("\t") << "messageIDName   = <" << data.pMessageIdName << ">\n";
		std::cerr << std::string("\t") << "messageIdNumber = " << data.messageIdNumber << "\n";
		std::cerr << std::string("\t") << "message         = <" << data.pMessage << ">\n";
		if (0 < data.queueLabelCount)
		{
			std::cerr << std::string("\t") << "Queue Labels:\n";
			for (uint32_t i = 0; i < data.queueLabelCount; i++)
			{
				std::cerr << std::string("\t\t") << "labelName = <" << data.pQueueLabels[i].pLabelName << ">\n";
			}
		}
		if (0 < data.cmdBufLabelCount)
		{
			std::cerr << std::string("\t") << "CommandBuffer Labels:\n";
			for (uint32_t i = 0; i < data.cmdBufLabelCount; i++)
			{
				std::cerr << std::string("\t\t") << "labelName = <" << data.pCmdBufLabels[i].pLabelName << ">\n";
			}
		}
		if (0 < data.objectCount)
		{
			std::cerr << std::string("\t") << "Objects:\n";
			for (uint32_t i = 0; i < data.objectCount; i++)
			{
				std::cerr << std::string("\t\t") << "Object " << i << "\n";
				std::cerr << std::string("\t\t\t") << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(data.pObjects[i].objectType))
					<< "\n";
				std::cerr << std::string("\t\t\t") << "objectHandle = " << data.pObjects[i].objectHandle << "\n";
				if (data.pObjects[i].pObjectName)
				{
					std::cerr << std::string("\t\t\t") << "objectName   = <" << data.pObjects[i].pObjectName << ">\n";
				}
			}
		}
#endif
	}

	vk_device::vk_device(const device_desc& desc)
		: device(desc)
	{
		// create the vulkan instance
		constexpr uint32 k_api_version = 0u;
		const vector<string> instance_layers = vk_helpers::getInstanceLayers();
		const vector<string> instance_extensions = vk_helpers::getInstanceExtensions();
		m_vk_instance = vk_helpers::createInstance("app_name", "engine_name", 
			instance_layers,
			instance_extensions,
			k_api_version,
			&on_debug_message);

		// get the main physical device
		m_vk_physical_devices = m_vk_instance.enumeratePhysicalDevices();
		const vk::PhysicalDevice& main_physical_device = m_vk_physical_devices.front();

		// query queue family indices:
		m_graphics_queue_family_index = vk_helpers::findGraphicsQueueFamilyIndex(main_physical_device.getQueueFamilyProperties());

		// create the main logical device:
		const vk::Device& main_device = vk_helpers::createDevice(
			main_physical_device,
			m_graphics_queue_family_index,
			vk_helpers::getDeviceExtensions());
		m_vk_devices.push_back(main_device);
	}

	void vk_device::cleanup()
	{
	}

	// get info about physical devices:
	vector<physical_device_info> vk_device::get_gpu_infos()
	{
		vector<physical_device_info> result_infos{};

		for (size_t i = 0u; i < m_vk_physical_devices.size(); ++i)
		{

		}

		return result_infos;
	}

	memory_info vk_device::get_memory_info() const
	{
		return memory_info();
	}

	void vk_device::copy_descriptors(const descriptor_range& source, const descriptor_range& dest, const graphics::e_descriptor_heap_type& heap_type)
	{

	}

	void* vk_device::get_native()
	{
		return nullptr;
	}

	void vk_device::submit(commandbuffer* commandbuffer)
	{
		
	}

	queue* vk_device::create_queue(const queue_desc& desc)
	{
		vk::Queue queue = {};
		get_main_device().getQueue(0u, 0u, &queue);
		return new vk_queue(desc, queue);
	}

	swapchain* vk_device::create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc)
	{
		// get window info
		auto rect = window.get_rect_client();
		uint32 width = rect.get_width();
		uint32 height = rect.get_height();
		e_format format = e_format::rgba8;

		// modify copy based on window data
		swapchain_desc desc_copy = desc;
		desc_copy.m_dimensions.x = width;
		desc_copy.m_dimensions.y = height;
		desc_copy.m_format = format;

		swapchain_dependencies dependencies{ this, queue };
		vk_swapchain::dependencies vk_dependencies{ get_main_device(), get_main_gpu(), m_vk_instance };
		return new vk_swapchain(window, desc_copy, dependencies, vk_dependencies);
	}

	descriptor_heap* vk_device::create_descriptor_heap(const descriptor_heap::create_args& args)
	{
		return nullptr;
	}

	command_allocator* vk_device::create_graphics_allocator()
	{
		vk::CommandPool vkcommandpool = get_main_device().createCommandPool({{}, m_graphics_queue_family_index});
		return new vk_command_allocator(vkcommandpool);
	}

	commandlist* vk_device::create_graphics_command_list(command_allocator* allocator, pipeline* init_state)
	{
		vk::CommandPool* vkcommandpool = allocator->get_native<vk::CommandPool>();
		vk::CommandBuffer vkcommandlist = get_main_device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(*vkcommandpool, vk::CommandBufferLevel::ePrimary, 1u)).front();
		return new vk_commandlist(vkcommandlist);
	}

	commandbuffer* vk_device::create_commandbuffer()
	{
		return nullptr;
	}

	fence* vk_device::create_fence(uint64 init_value)
	{
		vk::FenceCreateInfo info{};
		return new vk_fence(get_main_device().createFence(info));
	}

	resource* vk_device::create_resource(const tex2D_desc& desc, const heap_desc& heap_desc)
	{
		return nullptr;
	}

	resource* vk_device::create_resource(const buffer_desc& desc, const heap_desc& heap_desc)
	{
		return nullptr;
	}

	render_target_view* vk_device::create_rtv(descriptor_heap* rtv_heap, resource* resource)
	{
		vk::ImageViewCreateInfo info{};
		info.flags;
		info.format = convert(resource->get_format());
		info.image = *resource->get_native<vk::Image>();
		info.viewType = vk::ImageViewType::e2D;
		info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u);

		vk::ImageView vkview = get_main_device().createImageView(info);
		return new vk_render_target_view(vkview);
	}

	render_target_view* vk_device::create_rtv(descriptor_handle handle, resource* resource)
	{
		return nullptr;
	}

	depth_stencil_view* vk_device::create_dsv(descriptor_heap* dsv_heap, resource* resource)
	{
		return nullptr;
	}

	depth_stencil_view* vk_device::create_dsv(descriptor_handle handle, resource* resource)
	{
		return nullptr;
	}

	shader_resource_view* vk_device::create_srv(descriptor_heap* irv_heap, resource* resource)
	{
		return nullptr;
	}

	shader_resource_view* vk_device::create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource)
	{
		return nullptr;
	}

	shader_resource_view* vk_device::create_buffer_srv(descriptor_heap* srv_heap, resource* resource)
	{
		return nullptr;
	}

	shader_resource_view* vk_device::create_buffer_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource)
	{
		return nullptr;
	}

	sampler_view* vk_device::create_sampview(descriptor_heap* samp_heap, resource* resource)
	{
		return nullptr;
	}

	sampler_view* vk_device::create_sampview(descriptor_handle handle, resource* resource)
	{
		return nullptr;
	}

	rootsignature* vk_device::create_rootsignature(const rootsignature_desc& desc)
	{
		return nullptr;
	}

	pipeline* vk_device::create_pipeline(rootsignature* rootsig, const pipeline_desc& desc)
	{
		return nullptr;
	}

	const vk::Device& vk_device::get_main_device() const
	{
		return m_vk_devices[0];
	}

	vk::Device& vk_device::get_main_device()
	{
		return m_vk_devices[0];
	}

	const vk::PhysicalDevice& vk_device::get_main_gpu() const
	{
		return m_vk_physical_devices[0];
	}

	vk::PhysicalDevice& vk_device::get_main_gpu()
	{
		return m_vk_physical_devices[0];
	}
}