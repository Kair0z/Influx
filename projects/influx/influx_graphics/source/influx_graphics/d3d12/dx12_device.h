#pragma once
#include "influx_graphics/device.h"
#include "core/pointer.h"

struct IDXGIFactory2;
struct IDXGIAdapter1;
struct ID3D12Device;

namespace influx::graphics
{
	class dx12_queue;
}

namespace influx::graphics
{
	class dx12_device final
		: public device
	{
	public:
		dx12_device(const device_desc&);

		uint64 get_descriptor_stride(e_descriptor_heap_type type) const;

		virtual void cleanup();

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
		IDXGIFactory2* mpdxgi_factory;
		vector<IDXGIAdapter1*> mpdxgi_adapters;
		vector<ID3D12Device*> mpdx_devices;

		uint64 m_rtv_stride{};
		uint64 m_dsv_stride{};
		uint64 m_sampler_stride{};
		uint64 m_srv_stride{};

		dx12_queue* m_dx_queue_graphics = nullptr;
		dx12_queue* m_dx_queue_compute = nullptr;
		dx12_queue* m_dx_queue_copy = nullptr;

		vector<base*> m_children;

		template <typename _t, typename _tret, typename ..._args>
		inline _tret* new_child(_args&&... args)
		{
			m_children.push_back(new _t(std::forward<_args>(args)...));
			return (_tret*)m_children.back();
		}
	};
}