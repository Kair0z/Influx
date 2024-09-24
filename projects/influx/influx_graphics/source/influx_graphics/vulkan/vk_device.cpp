#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_device.h"
#include "vk_headers.h"

// helpers
#include "influx_graphics/vulkan/vk_helpers.h"
#include "influx_graphics/vulkan/vk_conversion.h"

// subheaders
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
	vk_device::vk_device(const device_desc& desc)
		: device(desc)
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

	swapchain* vk_device::create_swapchain(queue* queue, const platform::window_handle& window, const swapchain_desc& desc)
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