#pragma once
#include "influx_graphics/device.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_device final
		: public device
	{
	public:
		vk_device(const device_desc&);
		virtual void cleanup() override;

		// creation:
		virtual queue* create_queue(const queue_desc& desc) override;
		virtual swapchain* create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc) override;
		virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args& args) override;
		virtual command_allocator* create_graphics_allocator() override;
		virtual commandlist* create_graphics_command_list(command_allocator* allocator, pipeline* init_state = nullptr) override;
		virtual commandbuffer* create_commandbuffer() override;
		virtual fence* create_fence(uint64 init_value = 0u) override;
		virtual resource* create_resource(const tex2D_desc& desc, const heap_desc& heap_desc = {}) override;
		virtual resource* create_resource(const buffer_desc& desc, const heap_desc& heap_desc = {}) override;
		virtual render_target_view* create_rtv(descriptor_heap* rtv_heap, resource* resource) override;
		virtual render_target_view* create_rtv(descriptor_handle handle, resource* resource) override;
		virtual depth_stencil_view* create_dsv(descriptor_heap* dsv_heap, resource* resource) override;
		virtual depth_stencil_view* create_dsv(descriptor_handle handle, resource* resource) override;
		virtual shader_resource_view* create_srv(descriptor_heap* irv_heap, resource* resource) override;
		virtual shader_resource_view* create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) override;
		virtual shader_resource_view* create_buffer_srv(descriptor_heap* srv_heap, resource* resource) override;
		virtual shader_resource_view* create_buffer_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) override;
		virtual sampler_view* create_sampview(descriptor_heap* samp_heap, resource* resource) override;
		virtual sampler_view* create_sampview(descriptor_handle handle, resource* resource) override;
		virtual rootsignature* create_rootsignature(const rootsignature_desc& desc) override;
		virtual pipeline* create_pipeline(rootsignature* rootsig, const pipeline_desc& desc) override;

		// misc:
		virtual vector<physical_device_info> get_gpu_infos() override;
		virtual memory_info get_memory_info() const override;
		virtual void copy_descriptors(const descriptor_range& source, const descriptor_range& dest, const graphics::e_descriptor_heap_type& heap_type);
		virtual void* get_native() override final;
		virtual void submit(commandbuffer* commandbuffer) override;

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