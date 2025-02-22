#pragma once

// influx::graphics
#include "influx_graphics/device.h"

// influx::core
#include "core/pointer.h"

struct IDXGIFactory2;
struct IDXGIAdapter1;
struct ID3D12Device;
struct ID3D12CommandAllocator;
enum D3D12_COMMAND_LIST_TYPE;

namespace influx::graphics
{
	class dx12_queue;
	class dx12_base;
}

namespace influx::graphics
{
	class base;

	class dx12_device final : public device
	{
	public:
		dx12_device(const device_desc&);

		uint64 get_descriptor_stride(e_descriptor_heap_type type) const;

		virtual void release(base*) override;
		virtual void cleanup() override;

		// creation:
		virtual ptr<queue> create_queue(const queue_desc& desc) override;
		virtual ptr<swapchain> create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc) override;
		virtual ptr<descriptor_heap> create_descriptor_heap(const descriptor_heap::create_args& args) override;

		virtual ptr<commandlist> create_graphics_commandlist(graphics_pipeline* init_state = nullptr) override;
		virtual ptr<commandlist> create_compute_commandlist(compute_pipeline* init_state = nullptr) override;

		virtual ptr<fence> create_fence(uint64 init_value = 0u) override;
		virtual ptr<resource> create_resource(const struct cubemap_desc& desc, const heap_desc& heap_desc = {}) override;
		virtual ptr<resource> create_resource(const struct tex3D_desc& desc, const heap_desc& heap_desc = {}) override;
		virtual ptr<resource> create_resource(const tex2D_desc& desc, const heap_desc& heap_desc = {}) override;
		virtual ptr<resource> create_resource(const buffer_desc& desc, const heap_desc& heap_desc = {}) override;
		
		virtual ptr<resource> import_buffer(void* native_ptr, const buffer_desc& desc) override;
		virtual ptr<resource> import_texture(void* native_ptr, const tex2D_desc& desc) override;

		virtual void create_rtv(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_dsv(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_buffer_srv(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_buffer_uav(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_texture_srv(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_texture_uav(descriptor_handle cpu_handle, resource* resource) override;
		virtual void create_sampler_view(descriptor_handle cpu_handle, resource* resource) override;

		virtual ptr<rootsignature> create_rootsignature(const rootsignature_desc& desc) override;
		virtual ptr<graphics_pipeline> create_graphics_pipeline(rootsignature* rootsig, const graphics_pipeline_desc& desc) override;
		virtual ptr<compute_pipeline> create_compute_pipeline(rootsignature* rootsig, const compute_pipeline_desc& desc) override;

		// misc:
		virtual vector<physical_device_info> get_gpu_infos() override;
		virtual memory_info get_memory_info() const override;
		virtual void copy_descriptors(const descriptor_range& source, const descriptor_range& dest, const graphics::e_descriptor_heap_type& heap_type);
		virtual void* get_native() override final;

		ID3D12CommandAllocator* new_allocator(const D3D12_COMMAND_LIST_TYPE& type);
		void free_allocator(const D3D12_COMMAND_LIST_TYPE& type, ID3D12CommandAllocator*);
		
	private:
		IDXGIFactory2* mpdxgi_factory;
		vector<IDXGIAdapter1*> mpdxgi_adapters;
		vector<ID3D12Device*> mpdx_devices;

		struct command_alloc_entry final
		{
			ID3D12CommandAllocator* m_allocator = nullptr;
			bool m_in_flight = false;
		};
		vector<command_alloc_entry>& get_allocators(const D3D12_COMMAND_LIST_TYPE& type);
		vector<command_alloc_entry> m_direct_allocators;
		vector<command_alloc_entry> m_compute_allocators;
		vector<command_alloc_entry> m_copy_allocators;

		uint64 m_rtv_stride{};
		uint64 m_dsv_stride{};
		uint64 m_sampler_stride{};
		uint64 m_srv_stride{};

		dx12_queue* m_dx_queue_graphics = nullptr;
		dx12_queue* m_dx_queue_compute = nullptr;
		dx12_queue* m_dx_queue_copy = nullptr;

		vector<base*> m_children;
		vector<base*>::iterator find_child(base*);

		template <typename _nat, typename _ret, typename ..._args>
		_ret* new_child(_args&&... args)
		{
			_nat* new_child = new _nat(std::forward<_args&&>(args)...);
			m_children.push_back(new_child);
			return (_ret*)new_child;
		}
	};
}