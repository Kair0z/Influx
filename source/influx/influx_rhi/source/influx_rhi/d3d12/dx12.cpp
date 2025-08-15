#include "rhi_pch.h"
#include "influx_rhi.h"

#include "d3d12.h"
#include "dxgi1_6.h"
#include "d3dx12/d3dx12.h"
#include <D3Dcompiler.h>

namespace influx::rhi
{
	using dx12_factory		= IDXGIFactory2;
	using dx12_physdevice	= IDXGIAdapter1;
	using dx12_swapchain	= IDXGISwapChain4;
	using dx12_device		= ID3D12Device;
	using dx12_resource		= ID3D12Resource;
	using dx12_commandlist	= ID3D12GraphicsCommandList;
	using dx12_allocator	= ID3D12CommandAllocator;
	using dx12_descheap		= ID3D12DescriptorHeap;
	using dx12_fence		= ID3D12Fence;

	// [helpers]
	inline string hres_to_string(HRESULT hr)
	{
		char* msgBuf = nullptr;

		DWORD size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&msgBuf,
			0,
			nullptr);

		string result = (size && msgBuf) ? msgBuf : "Unknown error";
		LocalFree(msgBuf);
		return result;
	}
#define return_if_error(result_type, hres) \
	if (!::SUCCEEDED(hres)) return result_type::make_error(hres_to_string(hres));

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
		case e_descriptor_heap_type::dsv: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case e_descriptor_heap_type::resource: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case e_descriptor_heap_type::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		}
		return {};
	}
	D3D12_RESOURCE_STATES translate(e_resource_state state)
	{
		D3D12_RESOURCE_STATES result{};
		if (has_flag(state, e_resource_state::common))			result |= D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::rendertarget))	result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (has_flag(state, e_resource_state::present))			result |= D3D12_RESOURCE_STATE_PRESENT;
		return result;
	}
	e_descriptor_heap_type translate(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return e_descriptor_heap_type::rtv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return e_descriptor_heap_type::dsv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return e_descriptor_heap_type::resource;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return e_descriptor_heap_type::sampler;
		}
	}
	e_resource_type translate(D3D12_RESOURCE_DIMENSION type)
	{
		switch (type)
		{
		default:
		case D3D12_RESOURCE_DIMENSION_UNKNOWN: return {};
		case D3D12_RESOURCE_DIMENSION_BUFFER: return e_resource_type::buffer;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D: return e_resource_type::texture2D;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D: return e_resource_type::texture3D;
		}
	}
	result<uint32> query_descriptor_stride(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		return device->GetDescriptorHandleIncrementSize(type);
	}
	result<descriptor> sample_descheap(ID3D12DescriptorHeap* heap, uint32 stride, uint32 index, bool cpu)
	{
		if (cpu)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE base = heap->GetCPUDescriptorHandleForHeapStart();
			base.ptr += (index * stride);
			return static_cast<descriptor>(base.ptr);
		}
		else
		{
			D3D12_GPU_DESCRIPTOR_HANDLE base = heap->GetGPUDescriptorHandleForHeapStart();
			base.ptr += (index * stride);
			return static_cast<descriptor>(base.ptr);
		}
	}
	result<descriptor> sample_descheap(ID3D12Device* device, ID3D12DescriptorHeap* heap, uint32 index, bool cpu)
	{
		uint32 stride = query_descriptor_stride(device, heap->GetDesc().Type).get();
		return sample_descheap(heap, stride, index, cpu);
	}

	result<object_native> create_native(const device_desc& desc)
	{
		ID3D12Device* result{};
		dx12_physdevice* dxphys = (dx12_physdevice*)desc.m_physdevice;

		HRESULT res = ::D3D12CreateDevice(
			dxphys,
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
		// return_if_error(result<object_native>, res);

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
		
		IDXGIFactory2* factory = (IDXGIFactory2*)desc.m_device->m_data.m_factory;
		ID3D12CommandQueue* queue = (ID3D12CommandQueue*)desc.m_queue->m_native_object;

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
		return (dx12_swapchain*)int_swapchain;
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
		dx12_commandlist* commandlist{};
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

		commandlist->Close();

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

	result<object_native> create_native(const buffer_desc& desc)
	{
		ID3D12Device* device = (ID3D12Device*)desc.m_device;
		ID3D12Resource* resource = nullptr;

		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		HRESULT res = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return resource;
	}

	result<object_native> create_native(const texture2D_desc& desc)
	{
		ID3D12Device* device = (ID3D12Device*)desc.m_device;
		ID3D12Resource* resource = nullptr;

		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		dxdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		HRESULT res = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return resource;
	}

	result<object_native> create_native(const texture3D_desc& desc)
	{
		ID3D12Device* device = (ID3D12Device*)desc.m_device;
		ID3D12Resource* resource = nullptr;

		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		dxdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		HRESULT res = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return resource;
	}

	result<object_native> create_native(const pipeline_desc& desc)
	{
		return {};
	}

	result<object_native> create_native(const rootsignature_desc& desc)
	{
		return {};
	}

	result<buffer> import_buffer(object_native native)
	{
		ID3D12Resource* dxresource = (ID3D12Resource*)native;
		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();

		buffer imported{};
		imported.m_data.m_bytesize = desc.Width;
		imported.m_data.m_bytestride = 1u;
		imported.m_native_object = native;
		imported.m_desc.m_device = nullptr;
		return imported;
	}

	result<texture2D> import_texture2D(object_native native)
	{
		using result_type = result<texture2D>;

		ID3D12Resource* dxresource = (ID3D12Resource*)native;
		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();
		
		dx12_device* dxdevice = nullptr;
		HRESULT res = dxresource->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing resource");

		texture2D imported{};
		imported.m_native_object = native;
		imported.m_desc.m_device = dxdevice;
		return imported;
	}

	result<texture3D> import_texture3D(object_native native)
	{
		ID3D12Resource* dxresource = (ID3D12Resource*)native;
		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();

		texture3D imported{};
		imported.m_data;
		imported.m_native_object = native;
		imported.m_desc.m_device = nullptr;
		return imported;
	}

	result<descheap> import_descheap(object_native native)
	{
		using result_type = result<descheap>;

		dx12_descheap* dxheap = (dx12_descheap*)native;
		dx12_device* dxdevice = nullptr;
		HRESULT res = dxheap->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing heap");

		auto dxdesc = dxheap->GetDesc();		
		const D3D12_DESCRIPTOR_HEAP_TYPE dxtype = dxdesc.Type;
		const uint32 num_descriptors = dxdesc.NumDescriptors;

		descheap imported{};
		imported.m_data.m_desc_stride = query_descriptor_stride(dxdevice, dxtype);
		imported.m_desc.m_device = dxdevice;
		imported.m_desc.m_num_descriptors = num_descriptors;
		imported.m_desc.m_type = translate(dxtype);
		imported.m_native_object = native;
		imported.m_data.m_freelist.resize(num_descriptors, true);
		return imported;
	}
	
	result<commandlist> import_commandlist(object_native native)
	{
		using result_type = result<commandlist>;

		dx12_commandlist* dxcommandlist = (dx12_commandlist*)native;
		dx12_device* dxdevice = nullptr;
		HRESULT res = dxcommandlist->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		commandlist imported{};
		imported.m_desc.m_allocator;
		imported.m_desc.m_device;
		imported.m_desc.m_type;
		imported.m_native_object = native;
		imported.m_data.m_allocator;
		return imported;
	}
	
	result<command_allocator> import_allocator(object_native native)
	{
		using result_type = result<command_allocator>;

		dx12_allocator* dxallocator = (dx12_allocator*)native;
		dx12_device* dxdevice = nullptr;
		HRESULT res = dxallocator->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		command_allocator imported{};
		imported.m_desc.m_device = dxdevice;
		imported.m_desc.m_type;
		imported.m_native_object = native;
		return imported;
	}

	// [device]
	result<swapchain> device::create(const swapchain_desc& desc) const
	{
		using result_type = result<swapchain>;

		// override owning device
		auto cpy = desc; cpy.m_device = this;
		result_type result = rhi::create<rhi::swapchain>(desc);
		if (!result)
		{
			return result_type::make_error("failed creating native object");
		}

		// if specified, own a descriptor heap with rtvs to backbuffers
		if (desc.m_own_descriptors)
		{
			rhi::descheap_desc heap_desc{};
			heap_desc.m_device = m_native_object;
			heap_desc.m_num_descriptors = desc.m_num_buffers;
			heap_desc.m_type = rhi::e_descriptor_heap_type::rtv;
			auto rtv_heap = create_native(heap_desc);
			if (!rtv_heap)
			{
				return result_type::make_error("own_descriptors: failed creating descriptor heap for rtvs");
			}

			// set all rtvs as dirty
			for (uint32 i = 0u; i < desc.m_num_buffers; ++i)
				result.get().m_data.m_rtv_dirty_list.push_back(true);

			// store an rtv heap
			result.get().m_data.m_rtv_heap = rtv_heap.get();
		}

		return result;
	}
	result<queue> device::create(const queue_desc& desc) const
	{
		auto cpy = desc; cpy.m_device = this->m_native_object;
		return rhi::create<rhi::queue>(cpy);
	}
	result<command_allocator> device::create(const commandallocator_desc& desc) const
	{
		using result_type = result<command_allocator>;
		auto cpy = desc; cpy.m_device = this->m_native_object;

		// create native & import
		auto native_object = create_native(cpy);
		if (!native_object)
			return result_type::make_error("create command_allocator: failed creating native object!");

		auto res = import<command_allocator>(native_object.get());
		if (!res)
			return result_type::make_error("import command_allocator: failed importing");

		res.get().m_desc = desc;
		return res;
	}
	result<commandlist> device::create(const commandlist_desc& desc) const
	{
		using result_type = result<commandlist>;
		auto cpy = desc; cpy.m_device = this->m_native_object;

		// if allocator is not provided, we need to create our own
		if (desc.m_allocator == nullptr)
		{
			commandallocator_desc alloc_desc{};
			alloc_desc.m_type = desc.m_type;
			alloc_desc.m_device = this->m_native_object;
			auto res = create_native(alloc_desc);
			if (!res) return result_type::make_error("failed creating allocator!");

			// store new allocator
			cpy.m_allocator = res.get();
		}

		// create native
		auto native_object = create_native(cpy);
		if (!native_object)
			return result_type::make_error("create commandlist: failed creating native object!");

		// import the native object
		auto res = import<commandlist>(native_object.get());
		if (!res) return result_type::make_error("failed import!");
		
		res.get().m_data.m_allocator = cpy.m_allocator;

		// create fence if instructed
		if (desc.m_own_fence)
		{
			fence_desc fence_desc{};
			fence_desc.m_device = m_native_object;
			fence_desc.m_init_value = 0u;
			auto fence = create_native(fence_desc);
			if (!fence)
				return result_type::make_error("create commandlist: failed creating fence!");

			// store new fence
			res.get().m_data.m_fence = fence.get();
		}
		res.get().m_desc = desc;
		return res;
	}
	result<descheap> device::create(const descheap_desc& desc) const
	{
		using result_type = result<descheap>;
		auto cpy = desc; cpy.m_device = this->m_native_object;

		// create native & import
		auto native_object = create_native(cpy);
		if (!native_object) 
			return result_type::make_error("create descheap: failed creating native object!");

		return import<descheap>(native_object.get());
	}
	result<device> device::create(const device_desc& desc)
	{
		using result_type = result<device>;

		device result{};

		// store a dxgi factory
		dx12_factory* dxfactory = nullptr;
		HRESULT res = ::CreateDXGIFactory2(0u, IID_PPV_ARGS(&dxfactory));
		result.m_data.m_factory = dxfactory;

		// query all physical devices, store the first
		vector<dx12_physdevice*> physical_devices{};
		constexpr bool prefer_performance = true;
		{
			IDXGIAdapter1* adapter = nullptr;
			IDXGIFactory6* factory6;
			if (SUCCEEDED(dxfactory->QueryInterface(IID_PPV_ARGS(&factory6))))
			{
				for (UINT adapterIndex = 0;
					SUCCEEDED(factory6->EnumAdapterByGpuPreference(
						adapterIndex,
						prefer_performance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
						IID_PPV_ARGS(&adapter)));
						++adapterIndex)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);

					const bool adapter_is_software = desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
					const bool adapter_supports_dx12 = SUCCEEDED(::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr));

					if (adapter_is_software)
					{
						// Don't select the Basic Render Driver adapter.
						// If you want a software adapter, pass in "/warp" on the command line.
						continue;
					}

					// Check to see whether the adapter supports Direct3D 12, but don't create the
					// actual device yet.
					if (adapter_supports_dx12)
					{
						physical_devices.push_back(adapter);
					}
				}
			}
		}
		result.m_data.m_physical_device = physical_devices[0];

		device_desc desc_copy = desc;
		if (desc.m_physdevice == nullptr)
		{
			desc_copy.m_physdevice = result.m_data.m_physical_device;
		}

		auto native_create_res = create_native(desc);
		if (!native_create_res)
			return result_type::make_error("failed creating native");

		// get the device
		dx12_device* dxdevice = (dx12_device*)native_create_res.get();
		result.m_native_object = dxdevice;

		// enable the debug layer
		if (desc.m_debug)
		{
			ID3D12Debug* debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			debugController->Release();

			ID3D12InfoQueue* info_queue;
			res = dxdevice->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (res == S_OK)
			{
				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
#if 0
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
					// Workarounds for debug layer issues on hybrid-graphics systems
					D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
					D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
					D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
#endif
				};
				D3D12_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				info_queue->AddStorageFilterEntries(&filter);
			}
		}
		
		// store descriptor strides:
		result.m_data.m_descriptor_strides[0] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		result.m_data.m_descriptor_strides[1] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		result.m_data.m_descriptor_strides[2] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		result.m_data.m_descriptor_strides[3] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		return result;
	}
	result<> device::create_rtv(const texture2D& texture, descriptor descriptor) const
	{
		dx12_device* dxdevice = (dx12_device*)m_native_object;
		dx12_resource* dxresource = (dx12_resource*)texture.m_native_object;

		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		dxdevice->CreateRenderTargetView(dxresource, &desc, dxdescriptor);

		return {};
	}

	// [fence - interface]
	result<> fence::queue_signal(uint64 signal_value, const queue& queue)
	{
		return queue.queue_signal(*this, signal_value);
	}

	// [queue - interface]
	result<> queue::submit(const vector<commandlist*>& commandlists) const
	{
		ID3D12CommandQueue* dxqueue = (ID3D12CommandQueue*)this->m_native_object;
		vector<ID3D12CommandList*> dxcommandlists{};
		for (const commandlist* list : commandlists)
		{
			dxcommandlists.push_back((ID3D12CommandList*)list->m_native_object);
		}

		dxqueue->ExecuteCommandLists(dxcommandlists.size(), dxcommandlists.data());

		// signal finish
		for (commandlist* list : commandlists)
		{
			if (list->has_fence())
			{
				uint32& complete_value = list->m_data.m_fence_complete_value;
				complete_value += 1u;
				queue_signal(list->m_data.m_fence, complete_value);
			}
		}

		return {};
	}
	result<> queue::queue_signal(const fence& fence, uint64 signal_value) const
	{
		return queue_signal(fence.m_native_object, signal_value);
	}
	result<> queue::queue_signal(object_native fence, uint64 signal_value) const
	{
		ID3D12CommandQueue* dxqueue = (ID3D12CommandQueue*)this->m_native_object;
		ID3D12Fence* dxfence = (ID3D12Fence*)fence;
		HRESULT res = dxqueue->Signal(dxfence, signal_value);
		return {};
	}

	// [swapchain - interface]
	result<> swapchain::present() const
	{
		dx12_swapchain* dxswapchain = (dx12_swapchain*)m_native_object;
		HRESULT res = dxswapchain->Present(0u, 0u);
		return {};
	}
	result<uint32> swapchain::get_current_backbuffer_index() const
	{
		dx12_swapchain* dxswapchain = (dx12_swapchain*)m_native_object;
		return dxswapchain->GetCurrentBackBufferIndex();
	}
	result<texture2D> swapchain::get_backbuffer_resource(uint32 index) const
	{
		dx12_swapchain* dxswapchain = (dx12_swapchain*)m_native_object;

		ID3D12Resource* buffer = nullptr;
		HRESULT res = dxswapchain->GetBuffer(index, IID_PPV_ARGS(&buffer));
		return import_texture2D(buffer);
	}
	result<texture2D> swapchain::get_backbuffer_resource() const
	{
		uint32 backbuffer_index = get_current_backbuffer_index().get();
		return get_backbuffer_resource(backbuffer_index);
	}
	result<> swapchain::resize(const math::uint2& new_dim)
	{
		// flag all rtvs as dirty
		for (uint32 i = 0u; i < m_data.m_rtv_dirty_list.size(); ++i)
			m_data.m_rtv_dirty_list[i] = true;

		dx12_swapchain* dxswapchain = (dx12_swapchain*)m_native_object;

		DXGI_SWAP_CHAIN_DESC dxdesc{};
		HRESULT res = dxswapchain->GetDesc(&dxdesc);

		res = dxswapchain->ResizeBuffers(
			dxdesc.BufferCount,
			dxdesc.BufferDesc.Width,
			dxdesc.BufferDesc.Height,
			dxdesc.BufferDesc.Format,
			dxdesc.Flags
		);

		return {};
	}
	bool swapchain::owns_rtvs() const
	{
		return m_desc.m_own_descriptors;
	}
	result<descriptor> swapchain::get_or_create_backbuffer_rtv(const device& device)
	{
		using result_type = result<descriptor>;
		if (!owns_rtvs())
			return result_type::make_error("this swapchain does not own its own rtvs!");

		dx12_device* dxdevice = (dx12_device*)device.m_native_object;
		dx12_descheap* dxheap = (dx12_descheap*)m_data.m_rtv_heap;

		uint32 backbuffer_index = get_current_backbuffer_index().get();
		result<texture2D> backbuffer = get_backbuffer_resource();
		result<descriptor> descriptor = sample_descheap(dxdevice, dxheap, backbuffer_index, true);
		if (!descriptor)
			return result_type::make_error("failed sampling descheap!");

		if (m_data.m_rtv_dirty_list[backbuffer_index] == true)
		{
			device.create_rtv(backbuffer.get(), descriptor.get());
			m_data.m_rtv_dirty_list[backbuffer_index] = false;
		}

		return descriptor;
	}
	
	// [commandlist - interface]
	result<> commandlist::start(device& device)
	{
		if (m_data.m_allocator == nullptr)
		{
			auto new_alloc_res = device.create(commandallocator_desc{});
			if (!new_alloc_res) return result<>::make_error("failed creating new allocator");
			return start(new_alloc_res.get());
		}
		else
		{
			command_allocator alloc = import_allocator(m_data.m_allocator).get();
			return start(alloc);
		}
	}
	result<> commandlist::start(const command_allocator& allocator)
	{
		ID3D12GraphicsCommandList* dxcommandlist = (ID3D12GraphicsCommandList*)m_native_object;
		ID3D12CommandAllocator* dxallocator = (ID3D12CommandAllocator*)allocator.m_native_object;

		HRESULT res = dxallocator->Reset();

		res = dxcommandlist->Close();

		ID3D12PipelineState* dxinitpipeline = NULL;
		res = dxcommandlist->Reset(dxallocator, dxinitpipeline);
		return {};
	}
	result<> commandlist::end()
	{
		ID3D12GraphicsCommandList* dxcommandlist = (ID3D12GraphicsCommandList*)m_native_object;
		HRESULT res = dxcommandlist->Close();
		return {};
	}
	result<> commandlist::submit(queue& queue)
	{
		return queue.submit({ this });
	}
	result<> commandlist::wait_for_finish() const
	{
		if (m_desc.m_own_fence && m_data.m_fence != nullptr)
		{
			const uint32 max_value = 64 * 1024 * 1024;
			uint32 i = 0u;

			dx12_fence* dxfence = (dx12_fence*)m_data.m_fence;
			uint32 complete_value = m_data.m_fence_complete_value;

			while (i < max_value)
			{
				const uint32 fence_value = dxfence->GetCompletedValue();
				if (fence_value >= complete_value) return {};

				i++;
			}
		}
		return {};
	}
	bool commandlist::has_fence() const
	{
		return m_desc.m_own_fence && m_data.m_fence != nullptr;
	}
	result<> commandlist::transition_resource(texture2D& resource, e_resource_state new_state)
	{
		dx12_resource* dxresource = (dx12_resource*)resource.m_native_object;
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		
		const e_resource_state old_state = resource.m_data.m_current_state;
		if (new_state == old_state)
			return result<>::make_error("transition to same state is no-op!");

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Transition.pResource = dxresource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags;
		barrier.Transition.StateAfter = translate(new_state);
		barrier.Transition.StateBefore = translate(old_state);
		barrier.Transition.Subresource = 0u;

		dxcmdlist->ResourceBarrier(1u, &barrier);
		resource.m_data.m_previous_state = old_state;
		resource.m_data.m_current_state = new_state;
	}
	result<> commandlist::clear_rtv(descriptor rtv, const clear& clear)
	{
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(rtv) };
		dxcmdlist->ClearRenderTargetView(dxdescriptor, clear.m_colour.data(), 0u, NULL);
		return {};
	}

	// [descheap - interface]
	result<uint32> descheap::allocate(uint32 num_descriptors)
	{
		using result_type = result<uint32>;

		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;

		auto& freelist = m_data.m_freelist;
		for (uint32 i = 0u; i < freelist.size(); ++i)
		{
			// skip non-free ones
			if (is_allocated(i)) continue;

			bool all_neighbours_free = true;
			for (uint32 x = 0u; x < num_descriptors; ++i)
			{
				all_neighbours_free &= is_allocated(i + x);
			}

			if (all_neighbours_free)
			{
				// set all allocated descriptors unfree
				for (uint32 x = 0u; x < num_descriptors; ++x)
					freelist[i + x] = false;

				// return base index
				return i;
			}
		}

		return result_type::make_error("no free ranges found!");
	}
	bool descheap::is_allocated(uint32 index) const
	{
		if (index >= m_data.m_freelist.size())
			return false;

		return m_data.m_freelist[index];
	}
	result<> descheap::free(const vector<descriptor_range>& ranges)
	{
		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;
		return {};
	}
	result<descriptor> descheap::get_cpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_desc_stride, index, true);
	}
	result<descriptor> descheap::get_gpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_desc_stride, index, false);
	}

	// [texture2D]
	result<> texture2D::transition(commandlist& cmdlist, e_resource_state new_state)
	{
		return cmdlist.transition_resource(*this, new_state);
	}
}
