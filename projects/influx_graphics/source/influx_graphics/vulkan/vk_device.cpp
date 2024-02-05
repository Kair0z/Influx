#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_device.h"
#include "vk_headers.h"

// helpers
#include "influx_graphics/vulkan/vk_helpers.h"
#include "influx_graphics/vulkan/vk_conversion.h"

// subheaders
#include "influx_graphics/vulkan/vk_commandqueue.h"
#include "influx_graphics/vulkan/vk_fence.h"
#include "influx_graphics/vulkan/vk_swapchain.h"
#include "influx_graphics/vulkan/vk_resource.h"
#include "influx_graphics/vulkan/vk_commandlist.h"
#include "influx_graphics/vulkan/vk_allocator.h"
#include "influx_graphics/vulkan/vk_resource_views.h"
#include "influx_graphics/vulkan/vk_descriptorheap.h"

namespace influx::graphics
{
	vk_device::vk_device()
		: device()
	{
		m_vk_instance = vk_helpers::createInstance("app_name", "engine_name", {}, vk_helpers::getInstanceExtensions(), 0u);

		// query physical devices:
		m_vk_physical_devices = m_vk_instance.enumeratePhysicalDevices();

		// query queue family indices:
		m_graphics_queue_family_index = vk_helpers::findGraphicsQueueFamilyIndex(m_vk_physical_devices.front().getQueueFamilyProperties());

		// create the main logical device:
		m_vk_devices.push_back(vk_helpers::createDevice(m_vk_physical_devices.front(), 
			m_graphics_queue_family_index, vk_helpers::getDeviceExtensions()) );
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

	// get interface to graphics object creation:
	command_queue* vk_device::create_command_queue(const command_queue_desc& desc)
	{
		vk::Queue queue = {};
		get_main_device().getQueue(0u, 0u, &queue);
		return new vk_commandqueue(desc, queue);
	}

	swapchain* vk_device::create_swapchain(command_queue* queue, const platform::window_handle& window, const swapchain_desc& desc)
	{
		// get window info
		auto rect = platform::get_windowrect_client<uint32>(window);
		uint32 width = rect.m_width_height.x;
		uint32 height = rect.m_width_height.y;
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

	command_list* vk_device::create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state)
	{
		vk::CommandPool* vkcommandpool = allocator->get_native<vk::CommandPool>();
		vk::CommandBuffer vkcommandlist = get_main_device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(*vkcommandpool, vk::CommandBufferLevel::ePrimary, 1u)).front();
		return new vk_commandlist(vkcommandlist);
	}

	fence* vk_device::create_fence()
	{
		vk::FenceCreateInfo info{};
		vk::Fence vkfence = get_main_device().createFence(info);
		return new vk_fence(vkfence);
	}

	resource* vk_device::create_resource(const tex2D_desc& desc)
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