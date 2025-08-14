#include "rhi_pch.h"
#include "influx_rhi.h"

#include "d3d12.h"
#include "dxgi1_6.h"
#include "d3dx12/d3dx12.h"
#include <D3Dcompiler.h>

namespace influx::rhi
{
	D3D12_COMMAND_LIST_TYPE translate(e_queue_type type)
	{
		switch (type)
		{
		default:
		case e_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}
	D3D12_COMMAND_LIST_TYPE translate(e_commandlist_type type)
	{
		switch (type)
		{
		default:
		case e_commandlist_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}
	D3D12_DESCRIPTOR_HEAP_TYPE translate(e_descriptor_heap_type type)
	{
		switch (type)
		{
		case e_descriptor_heap_type::rtv: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		}
		return {};
	}
	
	result<object_native> create_native(const device_desc& desc)
	{
		IUnknown* physdevice = nullptr;

		ID3D12Device* result{};
		HRESULT res = ::D3D12CreateDevice(
			(IUnknown*)physdevice,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&result));
		return result;
	}

	result<object_native> create_native(const queue_desc& desc)
	{
		ID3D12CommandQueue* queue{};
		ID3D12Device* device = (ID3D12Device*)desc.m_device;

		D3D12_COMMAND_QUEUE_DESC dxdesc{};
		dxdesc.Type = translate(desc.m_type);
		dxdesc.Priority = desc.m_priority;
		dxdesc.Flags;

		HRESULT res = device->CreateCommandQueue(&dxdesc, IID_PPV_ARGS(&queue));
		return queue;
	}

	result<object_native> create_native(const swapchain_desc& desc)
	{
		const uint32 width = desc.m_dimensions.x;
		const uint32 height = desc.m_dimensions.y;

		// create dx swapchain
		DXGI_SWAP_CHAIN_DESC1 dxdesc = {};
		dxdesc.BufferCount = desc.m_num_buffers;
		dxdesc.Width = width;
		dxdesc.Height = height;
		dxdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxdesc.SampleDesc.Count = 1;
		dxdesc.Flags;
		
		IDXGIFactory2* factory = (IDXGIFactory2*)desc.m_factory;
		ID3D12CommandQueue* queue = (ID3D12CommandQueue*)desc.m_queue;

		IDXGISwapChain4* swapchain = nullptr;
		IDXGISwapChain1* int_swapchain;
		HRESULT res = factory->CreateSwapChainForHwnd(
			queue,
			(HWND)desc.m_window,
			&dxdesc,
			nullptr,
			nullptr,
			&int_swapchain);

		// does not support fullscreen transitions...
		factory->MakeWindowAssociation((::HWND)desc.m_window, DXGI_MWA_NO_ALT_ENTER);
		return swapchain;
	}

	result<object_native> create_native(const descheap_desc& desc)
	{
		ID3D12Device* device = (ID3D12Device*)desc.m_device;

		ID3D12DescriptorHeap* descheap{};
		D3D12_DESCRIPTOR_HEAP_DESC dxdesc{};
		dxdesc.Flags;
		dxdesc.NumDescriptors = desc.m_num_descriptors;
		dxdesc.Type = translate(desc.m_type);
		dxdesc.NodeMask;
		HRESULT res = device->CreateDescriptorHeap(&dxdesc, IID_PPV_ARGS(&descheap));
		return descheap;
	}

	result<object_native> create_native(const commandallocator_desc& desc)
	{
		ID3D12Device* device = (ID3D12Device*)desc.m_device;
		
		ID3D12CommandAllocator* allocator{};
		HRESULT res = device->CreateCommandAllocator(translate(desc.m_type),
			IID_PPV_ARGS(&allocator));

		return allocator;
	}

	result<object_native> create_native(const commandlist_desc& desc)
	{
		ID3D12CommandList* commandlist{};
		ID3D12Device* device = (ID3D12Device*)desc.m_device;
		ID3D12CommandAllocator* allocator = (ID3D12CommandAllocator*)desc.m_allocator;

		if (desc.m_allocator == nullptr)
		{
			// todo
		}

		HRESULT res = device->CreateCommandList(
			0u, 
			translate(desc.m_type),
			allocator,
			nullptr,
			IID_PPV_ARGS(&commandlist));

		return commandlist;
	}
	
	result<object_native> create_native(const fence_desc& desc)
	{
		ID3D12Fence* fence{};
		ID3D12Device* device = (ID3D12Device*)desc.m_device;

		D3D12_FENCE_FLAGS flags{};
		HRESULT res = device->CreateFence(desc.m_init_value, flags,
			IID_PPV_ARGS(&fence));

		return fence;
	}

	result<object_native> create_native(const resource_desc& desc)
	{
		return {};
	}

	result<object_native> create_native(const pipeline_desc& desc)
	{
		return {};
	}

	result<object_native> create_native(const rootsignature_desc& desc)
	{
		return {};
	}

	// [fence - interface]
	result<> fence::queue_signal(uint64 signal_value, const queue& queue)
	{
		return queue.queue_signal(*this, signal_value);
	}

	// [queue - interface]
	result<> queue::submit(const vector<const commandlist*>& commandlists) const
	{
		ID3D12CommandQueue* dxqueue = (ID3D12CommandQueue*)this->m_native_object;
		vector<ID3D12CommandList*> dxcommandlists{};
		for (const commandlist* list : commandlists)
		{
			dxcommandlists.push_back((ID3D12CommandList*)list->m_native_object);
		}

		dxqueue->ExecuteCommandLists(dxcommandlists.size(), dxcommandlists.data());
		return {};
	}
	result<> queue::queue_signal(const fence& fence, uint64 signal_value) const
	{
		ID3D12CommandQueue* dxqueue = (ID3D12CommandQueue*)this->m_native_object;
		ID3D12Fence* dxfence = (ID3D12Fence*)fence.m_native_object;

		HRESULT res = dxqueue->Signal(dxfence, signal_value);
		return {};
	}

	// [commandlist - interface]
	result<> commandlist::start(device& device)
	{
		auto new_alloc_res = device.create(commandallocator_desc{});
		if (!new_alloc_res) return result<>::make_error("failed creating new allocator");

		return start(new_alloc_res.get());
	}
	result<> commandlist::start(const command_allocator& allocator)
	{
		ID3D12GraphicsCommandList* dxcommandlist = (ID3D12GraphicsCommandList*)m_native_object;
		ID3D12CommandAllocator* dxallocator = (ID3D12CommandAllocator*)m_native_object;

		ID3D12PipelineState* dxinitpipeline = nullptr;
		HRESULT res = dxcommandlist->Reset(dxallocator, dxinitpipeline);
		return {};
	}
	result<> commandlist::end()
	{
		ID3D12GraphicsCommandList* dxcommandlist = (ID3D12GraphicsCommandList*)m_native_object;
		HRESULT res = dxcommandlist->Close();
		
		return {};
	}

	// [descheap - interface]
	result<descriptor_range> descheap::allocate(uint32 num_descriptors)
	{
		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;
		return {};
	}
	result<> descheap::free(const vector<descriptor_range>& ranges)
	{
		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;
		return {};
	}
}
