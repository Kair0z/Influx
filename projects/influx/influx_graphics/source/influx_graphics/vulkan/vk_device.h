#pragma once
#include "influx_graphics/device.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_device final
		: public device
	{
	public:
		vk_device();

		// get info about physical devices:
		virtual vector<physical_device_info> get_gpu_infos() override;

		// get interface to graphics object creation:
		virtual command_queue* create_command_queue(const command_queue_desc& desc) override;

		virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window, const swapchain_desc& desc) override;

		virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args& args) override;

		virtual command_allocator* create_graphics_allocator() override;

		virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state = nullptr) override;

		virtual fence* create_fence() override;

		virtual resource* create_resource(const tex2D_desc& desc) override;

		virtual render_target_view* create_rtv(descriptor_heap* rtv_heap, resource* resource) override;

		virtual render_target_view* create_rtv(descriptor_handle handle, resource* resource) override;

	private:
		vk::Instance m_vk_instance;
		vector<vk::PhysicalDevice> m_vk_physical_devices;
		vector<vk::Device> m_vk_devices;

		const vk::Device& get_main_device() const;
		vk::Device& get_main_device();

		const vk::PhysicalDevice& get_main_gpu() const;
		vk::PhysicalDevice& get_main_gpu();

		uint32_t m_graphics_queue_family_index = 0u;
	};
}