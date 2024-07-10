#pragma once
#include "influx_graphics/device.h"

struct IDXGIFactory2;
struct IDXGIAdapter1;
struct ID3D12Device;

namespace influx::graphics
{
	class dx12_device final
		: public device
	{
	public:
		dx12_device();

		uint64 get_descriptor_stride(e_descriptor_heap_type type) const;

		// get info about physical devices:
		virtual vector<physical_device_info> get_gpu_infos() override;

		// get interface to graphics object creation:
		virtual command_queue* create_command_queue(const command_queue_desc& desc) override;

		virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window, const swapchain_desc& desc) override;

		virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args& args) override;

		virtual command_allocator* create_graphics_allocator() override;

		virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline* init_state = nullptr) override;

		virtual fence* create_fence(uint64 init_value = 0u) override;

		virtual resource* create_resource(const tex2D_desc& desc, const heap_desc& heap_desc = {}) override;

		virtual resource* create_resource(const buffer_desc& desc, const heap_desc& heap_desc = {}) override;

		virtual render_target_view* create_rtv(descriptor_heap* rtv_heap, resource* resource) override;
		virtual render_target_view* create_rtv(descriptor_handle handle, resource* resource) override;

		virtual input_resource_view* create_srv(descriptor_heap* irv_heap, resource* resource) override;
		virtual input_resource_view* create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) override;

		virtual sampler_view* create_sampview(descriptor_heap* samp_heap, resource* resource) override;
		virtual sampler_view* create_sampview(descriptor_handle handle, resource* resource) override;

		virtual rootsignature* create_rootsignature(const rootsignature_desc& desc) override;

		virtual pipeline* create_pipeline(rootsignature* rootsig, const pipeline_desc& desc) override;

	private:
		IDXGIFactory2* mpdxgi_factory;
		vector<IDXGIAdapter1*> mpdxgi_adapters;
		vector<ID3D12Device*> mpdx_devices;

		uint64 m_rtv_stride{};
		uint64 m_dsv_stride{};
		uint64 m_sampler_stride{};
		uint64 m_cbv_stride{};
	};
}