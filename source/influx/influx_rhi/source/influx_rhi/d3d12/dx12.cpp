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
	using dx12_queue		= ID3D12CommandQueue;
	using dx12_swapchain	= IDXGISwapChain4;
	using dx12_device		= ID3D12Device;
	using dx12_resource		= ID3D12Resource;
	using dx12_commandlist	= ID3D12GraphicsCommandList;
	using dx12_allocator	= ID3D12CommandAllocator;
	using dx12_descheap		= ID3D12DescriptorHeap;
	using dx12_fence		= ID3D12Fence;

	template <typename _t, typename _p>
	inline result<_t*> cast(_p* ptr)
	{
		if (ptr == nullptr)
			return result<_t*>::make_error("cannot cast when ptr is nullptr!");

		_t* res = (_t*)ptr;
		if (res) return res;
		else return result<_t*>::make_error("failed casting ptr to type!");
	}

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
	template <class _t = char>
	inline result<_t> hres_to_result(HRESULT hres, const _t& value_if_success)
	{
		using result_type = result<_t>;
		if (SUCCEEDED(hres) == false)
			return result_type::make_error(hres_to_string(hres).c_str());

		return value_if_success;
	}

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
		case e_descriptor_heap_type::rsc: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case e_descriptor_heap_type::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		}
		return {};
	}
	D3D12_RESOURCE_STATES translate(e_resource_state state)
	{
		D3D12_RESOURCE_STATES result{};
		if (has_flag(state, e_resource_state::common))			result |= D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::render_target))	result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (has_flag(state, e_resource_state::present))			result |= D3D12_RESOURCE_STATE_PRESENT;
		return result;
	}
	e_descriptor_heap_type translate(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return e_descriptor_heap_type::rtv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return e_descriptor_heap_type::dsv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return e_descriptor_heap_type::rsc;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return e_descriptor_heap_type::sampler;
		}
		return e_descriptor_heap_type::num;
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

	static constexpr uint32 k_num_dxgi_formats = 256u;
	class static_pixel_formats final
	{
	private:
		static bool m_initialized;
		static std::pair<const char*, pixelformat> m_formats[k_num_dxgi_formats];
		static uint32 m_num_supported_formats;

		static void initialize();

	public:
		static uint32 get_num_supported_formats()
		{
			if (!m_initialized) initialize();
			return m_num_supported_formats;
		}

		static std::pair<const char*, pixelformat> const* get_formats()
		{
			if (!m_initialized) initialize();
			return m_formats;
		}
	};
	bool static_pixel_formats::m_initialized = false;
	uint32 static_pixel_formats::m_num_supported_formats = 0u;
	std::pair<const char*, pixelformat> static_pixel_formats::m_formats[k_num_dxgi_formats]{};

	uint32 get_num_supported_pixel_formats()
	{
		return static_pixel_formats::get_num_supported_formats();
	}
	DXGI_FORMAT translate(const pixelformat& format);
	uint32 get_translated_pixelformat(const pixelformat& format)
	{
		return translate(format);
	}
	const char* get_pixelformat_string(const pixelformat& format)
	{
		return static_pixel_formats::get_formats()[translate(format)].first;
	}
	const pixelformat& translate(DXGI_FORMAT format)
	{
		return static_pixel_formats::get_formats()[format].second;
	}
	DXGI_FORMAT translate(const pixelformat& format)
	{
		using namespace format;

		// find the translated DXGI format that matches ours
		// could be slow tbf...
		for (uint32 i = 0u; i < k_num_dxgi_formats; ++i)
		{
			DXGI_FORMAT as_dxgi = (DXGI_FORMAT)i;
			const pixelformat& form = translate(as_dxgi);
			if (format == form)
			{
				return as_dxgi;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
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

	result<object_native> create_native(const device_create_args& args)
	{
		using result_type = result<object_native>;

		if (args.m_physdevice == nullptr)
			return result_type::make_error("desc.m_physdevice is nullptr!");

		auto physdevice = cast<dx12_physdevice>(args.m_physdevice);
		if (!physdevice) 
			return result_type::make_error("args.m_physdevice failed casting to dx12_physdevice!");

		dx12_device* device{};
		HRESULT hres = ::D3D12CreateDevice(
			physdevice.get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device));
		
		return hres_to_result<object_native>(hres, device);
	}

	result<object_native> create_native(const queue_create_args& args)
	{
		using result_type = result<object_native>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_COMMAND_QUEUE_DESC dxdesc{};
		dxdesc.Type = translate(args.m_type);
		dxdesc.Priority = args.m_priority;
		dxdesc.Flags;

		ID3D12CommandQueue* queue{};
		HRESULT hres = device->CreateCommandQueue(&dxdesc, IID_PPV_ARGS(&queue));
		return hres_to_result<object_native>(hres, queue);
	}

	result<object_native> create_native(const swapchain_create_args& args)
	{
		using result_type = result<object_native>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");
		if (args.m_queue == nullptr)
			return result_type::make_error("args.m_queue is nullptr!");
		if (args.m_dimensions.is_zero())
			return result_type::make_error("args.m_dimensions are invalid!");
		if (args.m_num_buffers <= 0u || args.m_num_buffers > 3)
			return result_type::make_error("args.m_num_buffers is not valid!");
		if (!swapchain::is_swapchain_format_supported(args.m_format))
			return result_type::make_error("args.m_format is not swapchain supported!");

		const uint32 width = args.m_dimensions.x;
		const uint32 height = args.m_dimensions.y;

		// create dx swapchain
		DXGI_SWAP_CHAIN_DESC1 dxdesc = {};
		dxdesc.BufferCount = args.m_num_buffers;
		dxdesc.Width = width;
		dxdesc.Height = height;
		dxdesc.Format = translate(args.m_format);
		dxdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxdesc.SampleDesc.Count = 1;
		dxdesc.Flags;
		
		auto dxfactory = cast<dx12_factory>(args.m_device->m_data.m_factory);
		if (!dxfactory)
			return result_type::make_error("device carries no owning factory to create swapchain!");

		auto dxqueue = cast<dx12_queue>(args.m_queue->m_native_object);
		if (!dxqueue)
			return result_type::make_error("failed casting queue to dx12_queue!");

		IDXGISwapChain1* int_swapchain;
		HRESULT hres = dxfactory->CreateSwapChainForHwnd(
			dxqueue.get(),
			(HWND)args.m_window,
			&dxdesc,
			nullptr,
			nullptr,
			&int_swapchain);
		dx12_swapchain* swapchain = (dx12_swapchain*)int_swapchain;

		auto swapchain_create_res = hres_to_result<object_native>(hres, swapchain);
		if (!swapchain_create_res)
			return result_type::make_error("failed creating swapchain for Hwnd!");

		// associate the swapchain with the passed window
		hres = dxfactory->MakeWindowAssociation((::HWND)args.m_window, DXGI_MWA_NO_ALT_ENTER);
		if (!hres_to_result<object_native>(hres, swapchain))
		{
			return result_type::make_warning(swapchain, "after creating the swapchain, failed making window association! (result is still valid)");
		}
		else
		{
			return swapchain;
		}
	}

	result<object_native> create_native(const descheap_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		ID3D12DescriptorHeap* descheap{};
		D3D12_DESCRIPTOR_HEAP_DESC dxdesc{};
		dxdesc.Flags;
		dxdesc.NumDescriptors = args.m_num_descriptors;
		dxdesc.Type = translate(args.m_type);
		dxdesc.NodeMask;
		
		HRESULT hres = device->CreateDescriptorHeap(&dxdesc, IID_PPV_ARGS(&descheap));
		return hres_to_result<object_native>(hres, descheap);
	}

	result<object_native> create_native(const commandallocator_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");
		
		ID3D12CommandAllocator* allocator{};
		HRESULT hres = device->CreateCommandAllocator(translate(args.m_type), IID_PPV_ARGS(&allocator));
		return hres_to_result<object_native>(hres, allocator);
	}

	result<object_native> create_native(const commandlist_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("desc.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("desc.m_device failed casting to dx12_device!");

		auto allocator = cast<dx12_allocator>(args.m_allocator);
		if (args.m_allocator == nullptr)
		{
			// todo
		}

		dx12_commandlist* commandlist{};
		HRESULT hres = device->CreateCommandList(
			0u, 
			translate(args.m_type),
			allocator.get(),
			nullptr,
			IID_PPV_ARGS(&commandlist));

		hres = commandlist->Close();

		return hres_to_result<object_native>(hres, commandlist);
	}
	
	result<object_native> create_native(const fence_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		ID3D12Fence* fence{};
		D3D12_FENCE_FLAGS flags{};
		HRESULT hres = device->CreateFence(args.m_init_value, flags, IID_PPV_ARGS(&fence));
		return hres_to_result<object_native>(hres, fence);
	}

	result<object_native> create_native(const buffer_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		ID3D12Resource* resource = nullptr;
		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		HRESULT hres = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return hres_to_result<object_native>(hres, resource);
	}

	result<object_native> create_native(const texture2D_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		dxdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		dx12_resource* resource = nullptr;
		HRESULT hres = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return hres_to_result<object_native>(hres, resource);
	}

	result<object_native> create_native(const texture3D_create_args& args)
	{
		using result_type = result<object_native>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_HEAP_PROPERTIES heap_props{};
		D3D12_HEAP_FLAGS heap_flags{};
		D3D12_RESOURCE_DESC dxdesc{};
		dxdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		D3D12_RESOURCE_STATES dxstates{};
		D3D12_CLEAR_VALUE dxclear{};

		ID3D12Resource* resource = nullptr;
		HRESULT hres = device->CreateCommittedResource(
			&heap_props,
			heap_flags,
			&dxdesc,
			dxstates,
			&dxclear,
			IID_PPV_ARGS(&resource));

		return hres_to_result<object_native>(hres, resource);
	}

	result<object_native> create_native(const pipeline_create_args& args)
	{
		return {};
	}

	result<object_native> create_native(const rootsignature_create_args& args)
	{
		return {};
	}

	result<buffer> import_buffer(object_native native)
	{
		using result_type = result<buffer>;

		auto dxresource = cast<dx12_resource>(native);
		if (!dxresource)
			return result_type::make_error("native failed casting to dx12_resource!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();

		buffer imported{};
		imported.m_data.m_bytesize = desc.Width;
		imported.m_data.m_bytestride = 1u;
		imported.m_native_object = native;
		imported.m_create_args.m_device = nullptr;
		return imported;
	}

	result<texture2D> import_texture2D(object_native native)
	{
		using result_type = result<texture2D>;

		auto dxresource = cast<dx12_resource>(native);
		if (!dxresource)
			return result_type::make_error("native failed casting to dx12_resource!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();
		
		dx12_device* dxdevice = nullptr;
		HRESULT hres = dxresource->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr || hres != S_OK)
			return result_type::make_error("couldn't fetch owner device from existing resource");

		texture2D imported{};
		imported.m_native_object = native;
		imported.m_create_args.m_device = dxdevice;
		return imported;
	}

	result<texture3D> import_texture3D(object_native native)
	{
		using result_type = result<texture3D>;

		auto dxresource = cast<dx12_resource>(native);
		if (!dxresource)
			return result_type::make_error("native failed casting to dx12_resource!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();

		texture3D imported{};
		imported.m_data;
		imported.m_native_object = native;
		imported.m_create_args.m_device = nullptr;
		return imported;
	}

	result<descheap> import_descheap(object_native native)
	{
		using result_type = result<descheap>;

		auto dxheap = cast<dx12_descheap>(native);
		if (!dxheap)
			return result_type::make_error("native failed casting to dx12_descheap!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxheap->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing heap");

		auto dxdesc = dxheap->GetDesc();		
		const D3D12_DESCRIPTOR_HEAP_TYPE dxtype = dxdesc.Type;
		const uint32 num_descriptors = dxdesc.NumDescriptors;

		descheap imported{};
		imported.m_data.m_descriptor_stride = query_descriptor_stride(dxdevice, dxtype);
		imported.m_create_args.m_device = dxdevice;
		imported.m_create_args.m_num_descriptors = num_descriptors;
		imported.m_create_args.m_type = translate(dxtype);
		imported.m_native_object = native;
		imported.m_data.m_freelist.resize(num_descriptors, true);
		return imported;
	}
	
	result<commandlist> import_commandlist(object_native native)
	{
		using result_type = result<commandlist>;

		auto dxcommandlist = cast<dx12_commandlist>(native);
		if (!dxcommandlist)
			return result_type::make_error("native failed casting to dx12_commandlist!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxcommandlist->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		commandlist imported{};
		imported.m_create_args.m_allocator;
		imported.m_create_args.m_device;
		imported.m_create_args.m_type;
		imported.m_native_object = native;
		imported.m_data.m_allocator;
		return imported;
	}
	
	result<command_allocator> import_allocator(object_native native)
	{
		using result_type = result<command_allocator>;

		auto dxallocator = cast<dx12_allocator>(native);
		if (!dxallocator)
			return result_type::make_error("native failed casting to dx12_allocator!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxallocator->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		command_allocator imported{};
		imported.m_create_args.m_device = dxdevice;
		imported.m_create_args.m_type;
		imported.m_native_object = native;
		return imported;
	}

	// [device]
	result<swapchain> device::create(const swapchain_create_args& args) const
	{
		using result_type = result<swapchain>;

		// override the creator device to 'this'
		auto cpy = args; cpy.m_device = this;
		result_type result = rhi::create<rhi::swapchain>(cpy);
		if (!result) return result_type::make_error(result);

		// if specified, own a descriptor heap with rtvs to backbuffers
		if (args.m_own_descriptors)
		{
			rhi::descheap_create_args heap_desc{};
			heap_desc.m_device = m_native_object;
			heap_desc.m_num_descriptors = args.m_num_buffers;
			heap_desc.m_type = rhi::e_descriptor_heap_type::rtv;
			auto rtv_heap = create_native(heap_desc);
			if (!rtv_heap)
			{
				return result_type::make_error("own_descriptors: failed creating descriptor heap for rtvs");
			}

			// set all rtvs as dirty
			for (uint32 i = 0u; i < args.m_num_buffers; ++i)
				result.get().m_data.m_rtv_dirty_list.push_back(true);

			// store an rtv heap
			result.get().m_data.m_rtv_heap = rtv_heap.get();
		}

		return result;
	}
	result<queue> device::create(const queue_create_args& args) const
	{
		auto cpy = args; cpy.m_device = this->m_native_object;
		return rhi::create<rhi::queue>(cpy);
	}
	result<command_allocator> device::create(const commandallocator_create_args& args) const
	{
		using result_type = result<command_allocator>;
		auto cpy = args; cpy.m_device = this->m_native_object;

		// create native & import
		auto native_object = create_native(cpy);
		if (!native_object)
			return result_type::make_error("create command_allocator: failed creating native object!");

		auto res = import<command_allocator>(native_object.get());
		if (!res)
			return result_type::make_error("import command_allocator: failed importing");

		res.get().m_create_args = args;
		return res;
	}
	result<commandlist> device::create(const commandlist_create_args& args) const
	{
		using result_type = result<commandlist>;
		auto cpy = args; cpy.m_device = this->m_native_object;

		// if allocator is not provided, we need to create our own
		if (args.m_allocator == nullptr)
		{
			commandallocator_create_args alloc_args{};
			alloc_args.m_type = args.m_type;
			alloc_args.m_device = this->m_native_object;
			auto res = create_native(alloc_args);
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
		if (!res) 
			return result_type::make_error("failed import!");
		
		res.get().m_data.m_allocator = cpy.m_allocator;

		// create fence if instructed
		if (args.m_own_fence)
		{
			fence_create_args fence_args{};
			fence_args.m_device = m_native_object;
			fence_args.m_init_value = 0u;
			auto fence = create_native(fence_args);
			if (!fence)
				return result_type::make_error("create commandlist: failed creating fence!");

			// store new fence
			res.get().m_data.m_fence = fence.get();
		}
		res.get().m_create_args = args;
		return res;
	}
	result<descheap> device::create(const descheap_create_args& args) const
	{
		using result_type = result<descheap>;
		auto cpy = args; cpy.m_device = this->m_native_object;

		// create native & import
		auto native_object = create_native(cpy);
		if (!native_object) 
			return result_type::make_error("create descheap: failed creating native object!");

		return import<descheap>(native_object.get());
	}
	result<device> device::create(const device_create_args& args)
	{
		using result_type = result<device>;

		// store a dxgi factory
		dx12_factory* dxfactory = nullptr;
		HRESULT hres = ::CreateDXGIFactory2(0u, IID_PPV_ARGS(&dxfactory));
		auto create_factory_res = hres_to_result<dx12_factory*>(hres, dxfactory);
		if (!create_factory_res)
			return result_type::make_error("failed creating DXGI factory!");

		// query all physical devices ...
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

		// ... store the first
		dx12_physdevice* dxphysdevice = physical_devices[0];

		// if we didn't specify a physical device, select one!
		device_create_args edited_args = args;
		if (args.m_physdevice == nullptr)
		{
			edited_args.m_physdevice = dxphysdevice;
		}

		// create the actual device
		auto native_create_res = create_native(edited_args);
		if (!native_create_res)
			return result_type::make_error("failed creating native device!");
		dx12_device* dxdevice = (dx12_device*)native_create_res.get();

		// enable the debug layer
		if (edited_args.m_debug)
		{
			ID3D12Debug* debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			debugController->Release();

			ID3D12InfoQueue* info_queue;
			hres = dxdevice->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (hres == S_OK)
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

		// build the result
		device device{};
		device.m_native_object = native_create_res.get();
		device.m_create_args = edited_args;

		// setup initial data:
		// - descriptor strides
		// - owning factory
		device.m_data.m_factory = dxfactory;
		device.m_data.m_descriptor_strides[0] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device.m_data.m_descriptor_strides[1] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		device.m_data.m_descriptor_strides[2] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device.m_data.m_descriptor_strides[3] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		return device;
	}

	result<> device::create_rtv(const texture2D& texture, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		if (descriptor == 0u)
			return result_type::make_error("descriptor is nullptr!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto format = texture.get_current_format();
		if (!format)
			return result_type::make_error("texture has an invalid format!");

		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice;
		desc.Format = translate(*format.get());
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateRenderTargetView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_dsv(const texture2D& texture, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto format = texture.get_current_format();
		if (!format)
			return result_type::make_error("texture has an invalid format!");

		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Format = translate(*format.get());
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateDepthStencilView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_sampview(const sampler& sampler, descriptor descriptor) const
	{
		return {};
	}
	result<> device::create_srv(const texture2D& texture, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto format = texture.get_current_format();
		if (!format)
			return result_type::make_error("texture has an invalid format!");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Texture2D.MipLevels;
		desc.Texture2D.MostDetailedMip;
		desc.Texture2D.PlaneSlice;
		desc.Texture2D.ResourceMinLODClamp;
		desc.Format = translate(*format.get());
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateShaderResourceView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_uav(const texture2D& texture, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto format = texture.get_current_format();
		if (!format)
			return result_type::make_error("texture has an invalid format!");

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice;
		desc.Format = translate(*format.get());
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateUnorderedAccessView(resource.get(), nullptr, &desc, dxdescriptor);
		return {};
	}
	result<> device::create_srv(const buffer& buffer, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(buffer.m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.Flags;
		desc.Buffer.NumElements = (uint32)buffer.get_num_elements();
		desc.Buffer.StructureByteStride = (uint32)buffer.get_bytestride();
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateShaderResourceView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_uav(const buffer& buffer, descriptor descriptor) const
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(buffer.m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.CounterOffsetInBytes;
		desc.Buffer.Flags;
		desc.Buffer.NumElements = (uint32)buffer.get_num_elements();
		desc.Buffer.StructureByteStride = (uint32)buffer.get_bytestride();
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(descriptor) };
		device->CreateUnorderedAccessView(resource.get(), nullptr, &desc, dxdescriptor);
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
		using result_type = result<>;

		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");
		auto dxqueue = cast<dx12_queue>(m_native_object);
		if (!dxqueue)
			return result_type::make_error("m_native_object failed casting to dx12_queue!");

		vector<ID3D12CommandList*> dxcommandlists{};
		for (const commandlist* list : commandlists)
		{
			dxcommandlists.push_back((ID3D12CommandList*)list->m_native_object);
		}

		dxqueue->ExecuteCommandLists((uint32)dxcommandlists.size(), dxcommandlists.data());

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
		using result_type = result<>;

		if (m_native_object == nullptr) 
			return result_type::make_error("m_native_object is nullptr!");

		auto dxqueue = cast<dx12_queue>(m_native_object);
		if (!dxqueue) return result_type::make_error("m_native_object failed casting to dx12_queue!");

		if (fence == nullptr)
			return result_type::make_error("fence is nullptr!");

		auto dxfence = cast<dx12_fence>(fence);
		if (!dxqueue) return result_type::make_error("fence failed casting to dx12_fence!");

		HRESULT hres = dxqueue->Signal(dxfence.get(), signal_value);
		return hres_to_result<>(hres, {});
	}

	// [swapchain - interface]
	result<> swapchain::present(const present_args& args) const
	{
		using result_type = result<>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

		HRESULT hres = dxswapchain->Present(args.m_sync_interval, args.m_flags);
		return hres_to_result(hres, {});
	}
	result<uint32> swapchain::get_current_backbuffer_index() const
	{
		using result_type = result<uint32>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

		return dxswapchain->GetCurrentBackBufferIndex();
	}
	result<texture2D> swapchain::get_backbuffer_resource(uint32 index) const
	{
		using result_type = result<texture2D>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

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
		return m_create_args.m_own_descriptors;
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
	bool swapchain::is_swapchain_format_supported(const pixelformat& format)
	{
		const vector<pixelformat>& formats = get_swapchain_supported_formats();
		for (const auto& lformat : formats)
		{
			if (lformat == format) return true;
		}
		return false;
	}
	const vector<pixelformat>& swapchain::get_swapchain_supported_formats()
	{
		static vector<pixelformat> formats{};
		static bool done_once = false;
		if (!done_once)
		{
			formats.push_back(pixelformat::rgba_8_unorm());
			done_once = true;
		}
		return formats;
	}

	// [commandlist - interface]
	result<> commandlist::start(device& device)
	{
		if (m_data.m_allocator == nullptr)
		{
			auto new_alloc_res = device.create(commandallocator_create_args{});
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
		if (m_create_args.m_own_fence && m_data.m_fence != nullptr)
		{
			const uint32 max_value = 64 * 1024 * 1024;
			uint32 i = 0u;

			dx12_fence* dxfence = (dx12_fence*)m_data.m_fence;
			uint32 complete_value = m_data.m_fence_complete_value;

			while (i < max_value)
			{
				const uint64 fence_value = dxfence->GetCompletedValue();
				if (fence_value >= complete_value) return {};

				i++;
			}
		}
		return {};
	}
	bool commandlist::has_fence() const
	{
		return m_create_args.m_own_fence && m_data.m_fence != nullptr;
	}
	result<> commandlist::transition_resource(resource& resource, e_resource_state new_state)
	{
		using result_type = result<>;

		dx12_resource* dxresource = (dx12_resource*)resource.get_native_resource();
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		
		const e_resource_state old_state = resource.get_resource_state();
		if (new_state == old_state)
			return result<>::make_error("transition to same state is considered a no-op!");

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Transition.pResource = dxresource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags;
		barrier.Transition.StateAfter = translate(new_state);
		barrier.Transition.StateBefore = translate(old_state);
		barrier.Transition.Subresource = 0u;

		dxcmdlist->ResourceBarrier(1u, &barrier);
		auto res = resource.set_state(new_state);
		if (!res) return result_type::make_error("failed updating resource state!");

		return {};
	}
	result<> commandlist::clear_rtv(descriptor rtv, const clear& clear)
	{
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = static_cast<SIZE_T>(rtv) };
		dxcmdlist->ClearRenderTargetView(dxdescriptor, clear.m_colour.data(), 0u, NULL);
		return {};
	}
	result<> commandlist::copy_resource(const resource& source, resource& dest)
	{
		using result_type = result<>;
		if (source.get_width() != dest.get_width())
			return result_type::make_error("cannot copy resources of different width!");
		if (source.get_height() != dest.get_height())
			return result_type::make_error("cannot copy resources of different height!");

		auto dxsource = cast<dx12_resource>(source.get_native_resource());
		if (!dxsource) return result_type::make_error("failed casting source, to dx12_resource");
		auto dxdest = cast<dx12_resource>(dest.get_native_resource());
		if (!dxdest) return result_type::make_error("failed casting dest to dx12_resource");
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist) return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->CopyResource(dxdest.get(), dxsource.get());
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
	result<> descheap::free(const descriptor& desc)
	{
		return {};
	}
	result<> descheap::free(const uint32 index)
	{
		using result_type = result<>;
		auto dxheap = cast<dx12_descheap>(m_native_object);
		if (!dxheap) return result_type::make_error("failed casting m_native to dx12_heap");

		m_data.m_freelist[index] = true;
		return {};
	}
	result<descriptor> descheap::get_cpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_descriptor_stride, index, true);
	}
	result<descriptor> descheap::get_gpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_descriptor_stride, index, false);
	}

	// [resource]
	result<> resource::transition(commandlist& cmdlist, e_resource_state new_state)
	{
		return cmdlist.transition_resource(*this, new_state);
	}

	// [buffer]
	uint64 buffer::get_num_elements() const
	{
		return get_bytesize() / get_bytestride();
	}
	uint64 buffer::get_bytesize() const
	{
		return m_data.m_bytesize;
	}
	uint64 buffer::get_bytestride() const
	{
		return m_data.m_bytestride;
	}

	// [texture2D]
	result<> texture2D::transition(commandlist& cmdlist, e_resource_state new_state)
	{
		return cmdlist.transition_resource(*this, new_state);
	}

	result<pixelformat const*> texture2D::get_current_format() const
	{
		using result_type = result<pixelformat const*>;
		return &m_data.m_format;
	}

	void static_pixel_formats::initialize()
	{
		if (!m_initialized)
		{
			using namespace format;
			m_initialized = true;
			m_formats[DXGI_FORMAT_UNKNOWN]					= {"DXGI_FORMAT_UNKNOWN", {}};
			m_formats[DXGI_FORMAT_R32G32B32A32_TYPELESS]	= {"DXGI_FORMAT_R32G32B32A32_TYPELESS",		{ e_format::typeless,	{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_FLOAT]		= {"DXGI_FORMAT_R32G32B32A32_FLOAT",		{ e_format::sfloat,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_UINT]		= {"DXGI_FORMAT_R32G32B32A32_UINT",			{ e_format::uint,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_SINT]		= {"DXGI_FORMAT_R32G32B32A32_SINT",			{ e_format::sint,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_TYPELESS]		= {"DXGI_FORMAT_R32G32B32_TYPELESS",		{ e_format::typeless,	{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_FLOAT]			= {"DXGI_FORMAT_R32G32B32_FLOAT",			{ e_format::sfloat,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_UINT]			= {"DXGI_FORMAT_R32G32B32_UINT",			{ e_format::uint,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_SINT]			= {"DXGI_FORMAT_R32G32B32_SINT",			{ e_format::sint,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_TYPELESS]	= {"DXGI_FORMAT_R16G16B16A16_TYPELESS",		{ e_format::typeless,	{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_FLOAT]		= {"DXGI_FORMAT_R16G16B16A16_FLOAT",		{ e_format::sfloat,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_UNORM]		= {"DXGI_FORMAT_R16G16B16A16_UNORM",		{ e_format::unorm,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_UINT]		= {"DXGI_FORMAT_R16G16B16A16_UINT",			{ e_format::uint,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_SNORM]		= {"DXGI_FORMAT_R16G16B16A16_SNORM",		{ e_format::snorm,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_SINT]		= {"DXGI_FORMAT_R16G16B16A16_SINT",			{ e_format::sint,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R32G32_TYPELESS]			= {"DXGI_FORMAT_R32G32_TYPELESS",			{ e_format::typeless,	{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_FLOAT]				= {"DXGI_FORMAT_R32G32_FLOAT",				{ e_format::sfloat,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_UINT]				= {"DXGI_FORMAT_R32G32_UINT",				{ e_format::uint,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_SINT]				= {"DXGI_FORMAT_R32G32_SINT",				{ e_format::sint,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G8X24_TYPELESS]		= {"DXGI_FORMAT_R32G8X24_TYPELESS",			{ e_format::typeless,	{_r,_32}, {_g,_8}, {_x,_24,typeless} }};
			m_formats[DXGI_FORMAT_D32_FLOAT_S8X24_UINT]		= {"DXGI_FORMAT_D32_FLOAT_S8X24_UINT",		{ e_format::sfloat,		{_d,_32}, {_s,_8}, {_x,_24,uint} }};
			m_formats[DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS] = {"DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS",	{ e_format::sfloat,		{_r,_32}, {_x,_8}, {_x,_24,typeless} }};
			m_formats[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT]	= {"DXGI_FORMAT_X32_TYPELESS_G8X24_UINT",	{ e_format::typeless,	{_x,_32}, {_g,_8}, {_x,_24,uint} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_TYPELESS]		= {"DXGI_FORMAT_R10G10B10A2_TYPELESS",		{ e_format::typeless,	{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_UNORM]		= {"DXGI_FORMAT_R10G10B10A2_UNORM",			{ e_format::unorm,		{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_UINT]			= {"DXGI_FORMAT_R10G10B10A2_UINT",			{ e_format::uint,		{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R11G11B10_FLOAT]			= {"DXGI_FORMAT_R11G11B10_FLOAT",			{ e_format::sfloat,		{_r,_11}, {_g,_11}, {_b,_10} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_TYPELESS]		= {"DXGI_FORMAT_R8G8B8A8_TYPELESS",			{ e_format::typeless,	{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_UNORM]			= {"DXGI_FORMAT_R8G8B8A8_UNORM",			{ e_format::unorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};

			m_formats[DXGI_FORMAT_R8G8B8A8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_UINT]			= {"", { e_format::uint,			{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_SNORM]			= {"", { e_format::snorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_SINT]			= {"", { e_format::sint,			{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R16G16_TYPELESS]			= {"", { e_format::typeless,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_FLOAT]				= {"", { e_format::sfloat,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_UNORM]				= {"", { e_format::unorm,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_UINT]				= {"", { e_format::uint,			{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_SNORM]				= {"", { e_format::snorm,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_SINT]				= {"", { e_format::sint,			{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R32_TYPELESS]				= {"", { e_format::typeless,		{_r,_32} }};
			m_formats[DXGI_FORMAT_D32_FLOAT]				= {"", { e_format::sfloat,		{_d,_32} }};
			m_formats[DXGI_FORMAT_R32_FLOAT]				= {"", { e_format::sfloat,		{_r,_32} }};
			m_formats[DXGI_FORMAT_R32_UINT]					= {"", { e_format::uint,			{_r,_32} }};
			m_formats[DXGI_FORMAT_R32_SINT]					= {"", { e_format::sint,			{_r,_32} }};
			m_formats[DXGI_FORMAT_R24G8_TYPELESS]			= {"", { e_format::typeless,		{_r,_24}, {_g,_8} }};
			m_formats[DXGI_FORMAT_D24_UNORM_S8_UINT]		= {"", { e_format::unorm,		{_d,_24}, {_s,_8,uint} }};
			m_formats[DXGI_FORMAT_R24_UNORM_X8_TYPELESS]	= {"", { e_format::unorm,		{_r,_24}, {_x,_8,typeless} }};
			m_formats[DXGI_FORMAT_X24_TYPELESS_G8_UINT]		= {"", { e_format::typeless,		{_x,_24}, {_g,_8,uint } }};
			m_formats[DXGI_FORMAT_R8G8_TYPELESS]			= {"", { e_format::typeless,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_UNORM]				= {"", { e_format::unorm,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_UINT]				= {"", { e_format::uint,			{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_SNORM]				= {"", { e_format::snorm,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_SINT]				= {"", { e_format::sint,			{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R16_TYPELESS]				= {"", { e_format::typeless,		{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_FLOAT]				= {"", { e_format::sfloat,		{_r,_16} }};
			m_formats[DXGI_FORMAT_D16_UNORM]				= {"", { e_format::unorm,		{_d,_16} }};
			m_formats[DXGI_FORMAT_R16_UNORM]				= {"", { e_format::unorm,		{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_UINT]					= {"", { e_format::uint,			{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_SNORM]				= {"", { e_format::snorm,		{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_SINT]					= {"", { e_format::sint,			{_r,_16} }};
			m_formats[DXGI_FORMAT_R8_TYPELESS]				= {"", { e_format::typeless,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_UNORM]					= {"", { e_format::unorm,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_UINT]					= {"", { e_format::uint,			{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_SNORM]					= {"", { e_format::snorm,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_SINT]					= {"", { e_format::sint,			{_r,_8} }};
			m_formats[DXGI_FORMAT_A8_UNORM]					= {"", { e_format::unorm,		{_a,_8} }};
			m_formats[DXGI_FORMAT_R1_UNORM]					= {"", { e_format::unorm,		{_r,_1} }};
			m_formats[DXGI_FORMAT_R9G9B9E5_SHAREDEXP]		= {"", { e_format::shared_exp,	{_r,_9}, {_g,_9}, {_b,_9}, {_e,_5 } }};
			m_formats[DXGI_FORMAT_R8G8_B8G8_UNORM]			= {"", { e_format::unorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_G8R8_G8B8_UNORM]			= {"", { e_format::unorm,		{_g,_8}, {_r,_8}, {_g,_8}, {_b,_8} }};
			m_formats[DXGI_FORMAT_BC1_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC1_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC1_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC2_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC2_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC2_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC3_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC3_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC3_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC4_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC4_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC4_SNORM]				= {"", { e_format::snorm,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC5_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC5_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC5_SNORM]				= {"", { e_format::snorm,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC6H_TYPELESS]			= {"", { e_format::typeless,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC6H_UF16]				= {"", { e_format::ufloat,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC6H_SF16]				= {"", { e_format::sfloat,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC7_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_BC7_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_BC7_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_B5G6R5_UNORM]				= {"", { e_format::unorm,		{_b,_5}, {_g,_6}, {_r,_5} }};
			m_formats[DXGI_FORMAT_B5G5R5A1_UNORM]			= {"", { e_format::unorm,		{_b,_5}, {_g,_5}, {_r,_5}, {_a,_1} }};
			m_formats[DXGI_FORMAT_B8G8R8A8_UNORM]			= {"", { e_format::unorm,		{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_UNORM]			= {"", { e_format::unorm,		{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] = {"",{ e_format::xr_bias,	{_r,_10},{_g,_10},{_b,_10},{_a,_2,unorm}}};
			m_formats[DXGI_FORMAT_B8G8R8A8_TYPELESS]		= {"", { e_format::typeless,		{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8A8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_TYPELESS]		= {"", { e_format::typeless,		{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_B4G4R4A4_UNORM]			= {"", { e_format::unorm,		{_b,_4}, {_g,_4}, {_r,_4}, {_a,_4} }};
			m_formats[DXGI_FORMAT_A4B4G4R4_UNORM]			= {"", { e_format::unorm,		{_a,_4}, {_b,_4}, {_g,_4}, {_r,_4} }};
			m_formats[DXGI_FORMAT_AYUV]						= {"", { e_spec_format::AYUV }};
			m_formats[DXGI_FORMAT_Y410]						= {"", { e_spec_format::Y410 }};
			m_formats[DXGI_FORMAT_Y416]						= {"", { e_spec_format::Y416 }};
			m_formats[DXGI_FORMAT_NV12]						= {"", { e_spec_format::NV12 }};
			m_formats[DXGI_FORMAT_P010]						= {"", { e_spec_format::P010 }};
			m_formats[DXGI_FORMAT_P016]						= {"", { e_spec_format::P016 }};
			m_formats[DXGI_FORMAT_420_OPAQUE]				= {"", { e_spec_format::OP420 }};
			m_formats[DXGI_FORMAT_YUY2]						= {"", { e_spec_format::YUY2 }};
			m_formats[DXGI_FORMAT_Y210]						= {"", { e_spec_format::Y210 }};
			m_formats[DXGI_FORMAT_Y216]						= {"", { e_spec_format::Y216 }};
			m_formats[DXGI_FORMAT_NV11]						= {"", { e_spec_format::NV11 }};
			m_formats[DXGI_FORMAT_AI44]						= {"", { e_spec_format::AI44 }};
			m_formats[DXGI_FORMAT_IA44]						= {"", { e_spec_format::IA44 }};
			m_formats[DXGI_FORMAT_P8]						= {"", { e_spec_format::P8 }};
			m_formats[DXGI_FORMAT_A8P8]						= {"", { e_spec_format::A8P8 }};
			m_formats[DXGI_FORMAT_P208]						= {"", { e_spec_format::P208 }};
			m_formats[DXGI_FORMAT_V208]						= {"", { e_spec_format::V208 }};
			m_formats[DXGI_FORMAT_V408]						= {"", { e_spec_format::V408 }};
			m_formats[DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE] = {"", { e_spec_format::sampler_feedback_minmip_opaque}};
			m_formats[DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE] = {"", { e_spec_format::sampler_feedback_mip_region_used_opaque}};
		}
	}
}
