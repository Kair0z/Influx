#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_device.h"
#include "dx12_headers.h"

// helpers
#include "influx_graphics/d3d12/dx12_helpers.h"

// subheaders
#include "influx_graphics/d3d12/dx12_commandqueue.h"
#include "influx_graphics/d3d12/dx12_fence.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "influx_graphics/d3d12/dx12_allocator.h"
#include "influx_graphics/d3d12/dx12_resource_views.h"
#include "influx_graphics/d3d12/dx12_descriptorheap.h"

// core win32
#include "core/platform/win32/win32_window.h"

namespace influx::graphics
{
	dx12_device::dx12_device()
		: device()
	{
		// create factory
		::CreateDXGIFactory2(0u, IID_PPV_ARGS(&mpdxgi_factory));

#if _DEBUG
		dx12helpers::set_debug_layer_enabled(true);
#endif

		// query adapters
		auto adapters = dx12helpers::get_hardware_adapters<IDXGIAdapter1>(mpdxgi_factory);
		for (size_t i = 0u; i < adapters.size(); ++i)
		{
			mpdxgi_adapters.push_back(adapters[i]);
		}

		// create 1 dx12 logical device of first adapter
		mpdx_devices.push_back(
			dx12helpers::create_logical_device<ID3D12Device>(mpdxgi_adapters[0u]));

		// query strides:
		auto strides = dx12helpers::query_descriptor_strides(mpdx_devices[0]);
		m_rtv_stride = strides.m_rtv;
		m_dsv_stride = strides.m_dsv;
		m_sampler_stride = strides.m_sampler;
		m_cbv_stride = strides.m_cbv;
	}

	uint64 dx12_device::get_descriptor_stride(e_descriptor_heap_type type) const
	{
		switch (type)
		{
		case e_descriptor_heap_type::rtv: return m_rtv_stride;
		case e_descriptor_heap_type::dsv: return m_dsv_stride;
		case e_descriptor_heap_type::cbv: return m_cbv_stride;
		case e_descriptor_heap_type::sampler: return m_sampler_stride;
		}

		return 0u;
	}

	// get info about physical devices:
	vector<physical_device_info> dx12_device::get_gpu_infos()
	{
		vector<physical_device_info> result_infos{};

		for (size_t i = 0u; i < mpdxgi_adapters.size(); ++i)
		{

		}

		return result_infos;
	}

	// get interface to graphics object creation:
	command_queue* dx12_device::create_command_queue(const command_queue_desc& desc)
	{
		auto dxcommandqueue = dx12helpers::create_command_queue(
			mpdx_devices[0u], convert(desc.m_type), static_cast<int>(desc.m_priority));

		return new dx12_commandqueue(desc, dxcommandqueue);
	}

	swapchain* dx12_device::create_swapchain(command_queue* queue, const platform::window_handle& window, const swapchain_desc& desc)
	{
		auto rect = platform::get_windowrect_client<uint32>(window);
		uint32 width = rect.m_width_height.x;
		uint32 height = rect.m_width_height.y;
		e_format format = e_format::rgba8;

		// create dx swapchain
		IDXGISwapChain4* dxswapchain = dx12helpers::create_swapchain<IDXGISwapChain4>(
			mpdxgi_factory, queue->get_native<ID3D12CommandQueue>(),
			(::HWND)window, width, height, convert(format), desc.m_num_buffers);

		swapchain_desc desc_copy = desc;
		desc_copy.m_dimensions.x = width;
		desc_copy.m_dimensions.y = height;
		desc_copy.m_format = format;
		
		swapchain_dependencies dependencies{ this, queue };
		return new dx12_swapchain(desc_copy, dependencies, dxswapchain);
	}

	descriptor_heap* dx12_device::create_descriptor_heap(const descriptor_heap::create_args& args)
	{
		auto dxheap = dx12helpers::create_descriptor_heap(mpdx_devices[0u], convert(args.m_type), args.m_capacity);
		return new dx12_descriptor_heap(args, dxheap, get_descriptor_stride(args.m_type));
	}

	command_allocator* dx12_device::create_graphics_allocator()
	{
		auto dxallocator = dx12helpers::create_command_allocator(mpdx_devices[0u], D3D12_COMMAND_LIST_TYPE_DIRECT);
		return new dx12_command_allocator(dxallocator);
	}

	command_list* dx12_device::create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state)
	{
		auto dxcommandlist = dx12helpers::create_command_list<ID3D12GraphicsCommandList>(mpdx_devices[0u],
			allocator->get_native<ID3D12CommandAllocator>(), D3D12_COMMAND_LIST_TYPE_DIRECT,
			init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);

		dxcommandlist->Close();

		return new dx12_commandlist(dxcommandlist);
	}

	fence* dx12_device::create_fence(uint64 init_value)
	{
		return new dx12_fence(dx12helpers::create_fence<ID3D12Fence>(mpdx_devices[0u], init_value));
	}

	resource* dx12_device::create_resource(const tex2D_desc& desc)
	{
		auto dxresource = dx12helpers::create_tex2d_resource<ID3D12Resource>(mpdx_devices[0u],
			convert(desc.m_format), desc.m_dimensions.x, desc.m_dimensions.y, desc.m_arraysize, desc.m_num_mips,
			desc.m_sample_count, convert(desc.m_flags), D3D12_RESOURCE_STATE_COMMON);

		return new dx12_resource(dxresource, desc);
	}

	render_target_view* dx12_device::create_rtv(descriptor_heap* rtv_heap, resource* resource)
	{
		ID3D12DescriptorHeap* heap = rtv_heap->get_native<ID3D12DescriptorHeap>();
		
		// allocate a new rtv descriptor:
		descriptor_handle new_descriptor = rtv_heap->allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(new_descriptor) };

		return create_rtv(new_descriptor, resource);
	}

	render_target_view* dx12_device::create_rtv(descriptor_handle handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(handle) };

		// create the rtv
		ID3D12Resource* dxresource = resource->get_native<ID3D12Resource>();
		auto dx_rtv = dx12helpers::create_rtv(mpdx_devices[0u], dxresource,
			new_dxdescriptor, convert(resource->get_format()));

		return new dx12_render_target_view(dx_rtv);
	}
}