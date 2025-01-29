#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_device.h"
#include "dx12_headers.h"

// helpers
#include "influx_graphics/d3d12/dx12_helpers.h"

// subheaders
#include "influx_graphics/d3d12/dx12_base.h"
#include "influx_graphics/d3d12/dx12_queue.h"
#include "influx_graphics/d3d12/dx12_fence.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "influx_graphics/d3d12/dx12_descriptors.h"
#include "influx_graphics/d3d12/dx12_pipeline.h"
#include "influx_graphics/d3d12/dx12_rootsignature.h"

// core win32
#include "core/platform/win32/win32_window.h"

namespace influx::graphics
{
	dx12_device::dx12_device(const device_desc& desc)
		: device(desc)
	{
		// create factory
		::CreateDXGIFactory2(0u, IID_PPV_ARGS(&mpdxgi_factory));

#if INFLUX_DEBUG
		dx12helpers::set_debug_layer_enabled(true);
#endif
		// query adapters
		auto adapters = dx12helpers::get_hardware_adapters<IDXGIAdapter1>(mpdxgi_factory);
		for (uint64 i = 0u; i < adapters.size(); ++i)
		{
			mpdxgi_adapters.push_back(adapters[i]);
		}

		// create 1 dx12 logical device of first adapter
		mpdx_devices.push_back(
			dx12helpers::create_logical_device<ID3D12Device>(mpdxgi_adapters[0u]));

#if INFLUX_DEBUG
		for (uint64 i = 0u; i < mpdx_devices.size(); ++i)
		{
			ID3D12InfoQueue* info_queue;
			HRESULT res = mpdx_devices[i]->QueryInterface(IID_PPV_ARGS(&info_queue));
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
#endif

		// query strides:
		auto strides = dx12helpers::query_descriptor_strides(mpdx_devices[0]);
		m_rtv_stride = strides.m_rtv;
		m_dsv_stride = strides.m_dsv;
		m_sampler_stride = strides.m_sampler;
		m_srv_stride = strides.m_cbv;

		// internal queues
		queue_desc queue_desc{};
		if (desc.m_has_graphics_queue)
		{
			queue_desc.m_type = e_queue_type::graphics;
			m_graphics_queue = create_queue(queue_desc);
			m_dx_queue_graphics = (dx12_queue*)m_graphics_queue;
		}

		if (desc.m_has_compute_queue)
		{
			queue_desc.m_type = e_queue_type::compute;
			m_compute_queue = create_queue(queue_desc);
			m_dx_queue_compute = (dx12_queue*)m_compute_queue;
		}

		if (desc.m_has_copy_queue)
		{
			queue_desc.m_type = e_queue_type::copy;
			m_copy_queue = create_queue(queue_desc);
			m_dx_queue_copy = (dx12_queue*)m_copy_queue;
		}
	}

	uint64 dx12_device::get_descriptor_stride(e_descriptor_heap_type type) const
	{
		switch (type)
		{
		case e_descriptor_heap_type::rtv: return m_rtv_stride;
		case e_descriptor_heap_type::dsv: return m_dsv_stride;
		case e_descriptor_heap_type::srv: return m_srv_stride;
		case e_descriptor_heap_type::sampler: return m_sampler_stride;
		}

		return 0u;
	}

	void dx12_device::release(base* child)
	{
		device::release(child);

		// remove pointer from bookkeeping
		auto found_child = find_child(child);
		if (found_child != m_children.cend())
		{
			m_children.erase(found_child);
		}
	}

	void dx12_device::cleanup()
	{
		// release the unreleased
		for (size_t i = 0u; i < m_children.size(); ++i)
		{
			if (m_children[i]->is_valid())
				device::release(m_children[i]);
		}
		m_children.clear();

		for (size_t i = 0u; i < mpdx_devices.size(); ++i)
		{
			if (mpdx_devices[i] != nullptr)
				mpdx_devices[i]->Release();
		}

		for (size_t i = 0u; i < mpdxgi_adapters.size(); ++i)
		{
			mpdxgi_adapters[i]->Release();
		}

		mpdxgi_factory->Release();

		set_initialized(false);
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

	memory_info dx12_device::get_memory_info() const
	{
		memory_info result_info{};

		DXGI_QUERY_VIDEO_MEMORY_INFO out_info{};
		HRESULT res = ((IDXGIAdapter3*)mpdxgi_adapters[0u])->QueryVideoMemoryInfo(0u,
			DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &out_info);

		if (res == S_OK)
		{
			result_info.m_gpu_budget = out_info.Budget;
			result_info.m_gpu_usage = out_info.CurrentUsage;
		}

		return result_info;
	}

	// get interface to graphics object creation:
	ptr<queue> dx12_device::create_queue(const queue_desc& desc)
	{
		int priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		switch (desc.m_priority)
		{
		case e_queue_priority::normal: priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; break;
		case e_queue_priority::high: priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH; break;
		case e_queue_priority::global_realtime: priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME; break;
		}

		auto dxcommandqueue = dx12helpers::create_queue(
			mpdx_devices[0u], translate(desc.m_type), priority);

		return new_child<dx12_queue, queue>(desc, dxcommandqueue);
	}

	ptr<swapchain> dx12_device::create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc)
	{
		const math::vectoru2 dimensions = window.get_dimensions(platform::window::e_space::client);
		const uint32 width = dimensions.x;
		const uint32 height = dimensions.y;
		e_format format = e_format::rgba8;

		// create dx swapchain
		IDXGISwapChain4* dxswapchain = dx12helpers::create_swapchain<IDXGISwapChain4>(
			mpdxgi_factory, queue->get_native<ID3D12CommandQueue>(),
			(::HWND)window.get_platform_handle(), width, height, translate(format), desc.m_num_buffers);

		swapchain_desc desc_copy = desc;
		desc_copy.m_dimensions.x = width;
		desc_copy.m_dimensions.y = height;
		desc_copy.m_format = format;
		
		swapchain_dependencies dependencies{ this, queue };
		return new_child<dx12_swapchain, swapchain>(desc_copy, dependencies, dxswapchain);
	}

	ptr<descriptor_heap> dx12_device::create_descriptor_heap(const descriptor_heap::create_args& args)
	{
		auto dxheap = dx12helpers::create_descriptor_heap(mpdx_devices[0u], translate(args.m_type), args.m_capacity, args.m_shader_visible);
		return new_child<dx12_descriptor_heap, descriptor_heap>(args, dxheap, get_descriptor_stride(args.m_type));
	}

	ptr<commandlist> dx12_device::create_commandlist(e_commandlist_type type, detail::pipeline* init_state)
	{
		D3D12_COMMAND_LIST_TYPE dxtype = translate(type);
		ID3D12CommandAllocator* dxallocator = new_allocator(dxtype);
		ID3D12PipelineState* dxpipeline = (init_state != nullptr) 
			? init_state->get_native<ID3D12PipelineState>() : nullptr;

		commandlist* result = nullptr;
		switch (type)
		{
		case e_commandlist_type::graphics:
		{
			ID3D12GraphicsCommandList* dxcommandlist = dx12helpers::create_command_list<ID3D12GraphicsCommandList>(
				mpdx_devices[0u], dxallocator, dxtype, dxpipeline);
			dxcommandlist->Close();

			return new_child<dx12_commandlist, commandlist>(dxcommandlist, dxallocator);
		}
		break;

		case e_commandlist_type::compute:
		{
			ID3D12GraphicsCommandList* dxcommandlist = dx12helpers::create_command_list<ID3D12GraphicsCommandList>(
				mpdx_devices[0u], dxallocator, dxtype, dxpipeline);
			dxcommandlist->Close();

			return new_child<dx12_commandlist, commandlist>(dxcommandlist, dxallocator);
		}
		break;
		}
		
		influx_assert(false);
		return nullptr;
	}

	ptr<commandlist> dx12_device::create_graphics_commandlist(detail::pipeline* init_state)
	{
		return create_commandlist(e_commandlist_type::graphics, init_state);
	}

	ptr<commandlist> dx12_device::create_compute_commandlist(detail::pipeline* init_state)
	{
		return create_commandlist(e_commandlist_type::compute, init_state);
	}

	ptr<fence> dx12_device::create_fence(uint64 init_value)
	{
		return new_child<dx12_fence, fence>(dx12helpers::create_fence<ID3D12Fence>(mpdx_devices[0u], init_value));
	}

	ptr<resource> dx12_device::create_resource(const tex2D_desc& desc, const heap_desc& heap_desc)
	{
		auto dxresource = dx12helpers::create_tex2d_resource<ID3D12Resource>(mpdx_devices[0u],
			translate(heap_desc.m_type),
			translate(desc.m_format), 
			desc.m_dimensions.x, desc.m_dimensions.y, 
			desc.m_arraysize, desc.m_num_mips,
			desc.m_sample_count, 
			translate(desc.m_flags), 
			translate(desc.m_init_state));

		return new_child<dx12_resource, resource>(dxresource, desc);
	}

	ptr<resource> dx12_device::create_resource(const buffer_desc& desc, const heap_desc& heap_desc)
	{
		auto dxresource = dx12helpers::create_buffer_resource<ID3D12Resource>(mpdx_devices[0], 
			translate(heap_desc.m_type),
			desc.m_bytesize, 
			translate(desc.m_flags), 
			translate(desc.m_init_state));

		return new_child<dx12_resource, resource>(dxresource, desc);
	}

	ptr<resource> dx12_device::import_buffer(void* native_ptr, const buffer_desc& desc)
	{
		ID3D12Resource* dxresource = (ID3D12Resource*)native_ptr;
		return new_child<dx12_resource, resource>(dxresource, desc);
	}

	ptr<resource> dx12_device::import_texture(void* native_ptr, const tex2D_desc& desc)
	{
		ID3D12Resource* dxresource = (ID3D12Resource*)native_ptr;
		return new_child<dx12_resource, resource>(dxresource, desc);
	}

	void dx12_device::create_rtv(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor = { .ptr = (SIZE_T)cpu_handle };

		ID3D12Resource* dxresource = resource->get_native<ID3D12Resource>();
		dx12helpers::create_rtv(mpdx_devices[0u], dxresource, dxdescriptor, translate(resource->get_format()));
	}
	void dx12_device::create_dsv(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxdesc = { .ptr = (SIZE_T)cpu_handle };
		ID3D12Resource* dxresource = resource->get_native<ID3D12Resource>();
		auto dx_dsv = dx12helpers::create_dsv(mpdx_devices[0u], dxresource, dxdesc, translate(resource->get_format()));
	}
	void dx12_device::create_buffer_srv(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { (size_t)(cpu_handle) };

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.NumElements = resource->get_num_elements();
		srv_desc.Buffer.StructureByteStride = (uint32)resource->get_bytestride();
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		mpdx_devices[0u]->CreateShaderResourceView(
			resource->get_native<ID3D12Resource>(),
			&srv_desc, dxcpu_descriptor);
	}
	void dx12_device::create_buffer_uav(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { .ptr = (size_t)(cpu_handle) };

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer;
		mpdx_devices[0u]->CreateUnorderedAccessView(resource->get_native<ID3D12Resource>(), nullptr,
			&uav_desc, dxcpu_descriptor);
	}
	void dx12_device::create_texture_srv(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { .ptr = (size_t)(cpu_handle) };

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = translate(resource->get_format());
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;

		mpdx_devices[0u]->CreateShaderResourceView(resource->get_native<ID3D12Resource>(),
			&srv_desc, dxcpu_descriptor);
	}
	void dx12_device::create_texture_uav(descriptor_handle cpu_handle, resource* resource)
	{
		influx_assert(false);
	}
	void dx12_device::create_sampler_view(descriptor_handle cpu_handle, resource* resource)
	{
		D3D12_SAMPLER_DESC desc{};
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // Wrap addressing for U
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // Wrap addressing for V
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // Wrap addressing for W
		desc.MipLODBias = 0.0f; // No bias for MIP levels
		desc.MaxAnisotropy = 1; // No anisotropic filtering
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS; // No comparison
		// desc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.0f; // Black border color
		desc.MinLOD = 0.0f; // Minimum LOD
		desc.MaxLOD = D3D12_FLOAT32_MAX; // Maximum LOD

		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { .ptr = (size_t)(cpu_handle) };
		mpdx_devices[0u]->CreateSampler(&desc, dxcpu_descriptor);
	}

	ptr<rootsignature> dx12_device::create_rootsignature(const rootsignature_desc& desc)
	{
		ID3D12RootSignature* dxrootsignature = nullptr;

		// setup versioning
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (mpdx_devices[0]->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)) != S_OK)
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// build a name-to-param-idx map
		umap<string, uint32> name_to_param_idx{};

		// setup root parameters
		vector<CD3DX12_ROOT_PARAMETER1> root_parameters{};
		vector<CD3DX12_STATIC_SAMPLER_DESC> static_samplers{};
		vector<vector<CD3DX12_DESCRIPTOR_RANGE1>> root_descriptor_ranges(desc.m_resource_tables.size());

		// constants
		for (const root_param_constants& constants : desc.m_constants)
		{
			name_to_param_idx[constants.m_common.m_name] = (uint32)root_parameters.size();

			root_parameters.push_back({});
			root_parameters.back().InitAsConstants(constants.m_num_dwords, constants.m_common.m_shader_register, 
				constants.m_common.m_register_space, translate(constants.m_common.m_visibility));
		}

		// resources
		for (const root_param_resource& resource : desc.m_resources)
		{
			name_to_param_idx[resource.m_common.m_name] = (uint32)root_parameters.size();

			root_parameters.push_back({});
			switch (resource.m_type)
			{
			case root_param_resource::e_type::srv: 
				root_parameters.back().InitAsShaderResourceView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::cbv:
				root_parameters.back().InitAsConstantBufferView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::uav:
				root_parameters.back().InitAsUnorderedAccessView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;
			}
		}

		// resource tables
		size_t descriptor_table_idx = 0u;
		for (const root_param_resource_table& tables : desc.m_resource_tables)
		{
			name_to_param_idx[tables.m_common.m_name] = (uint32)root_parameters.size();

			vector<CD3DX12_DESCRIPTOR_RANGE1>& ranges = root_descriptor_ranges[descriptor_table_idx++];

			for (const root_param_resource_range& range : tables.m_resource_ranges)
			{
				D3D12_DESCRIPTOR_RANGE_TYPE range_type{};
				switch (range.m_type)
				{
				case root_param_resource_range::e_type::cbv: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
				case root_param_resource_range::e_type::uav: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
				case root_param_resource_range::e_type::srv: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
				case root_param_resource_range::e_type::sampler: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
				}

				ranges.push_back({});
				ranges.back().Init(range_type,
					range.m_num_resources, 
					range.m_shader_register,
					range.m_register_space);
			}

			root_parameters.push_back({});
			root_parameters.back().InitAsDescriptorTable((uint32)ranges.size(), ranges.data(), translate(tables.m_common.m_visibility));
		}

		// samplers
		for (const root_static_sampler& sampler : desc.m_static_samplers)
		{
			static_samplers.push_back({});
			static_samplers.back().Init(
				sampler.m_common.m_shader_register,
				translate(sampler.m_filter),
				translate(sampler.m_wrap_u),
				translate(sampler.m_wrap_v),
				translate(sampler.m_wrap_w),
				sampler.m_mip_lod_bias,
				sampler.m_max_anisotropy,
				translate(sampler.m_comparison_func),
				translate(sampler.m_border_color),
				sampler.m_min_lod,
				sampler.m_max_lod,
				translate(sampler.m_common.m_visibility));
		}

		
		// initialize the desc, and create the root signature
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

		rootSignatureDesc.Init_1_1(
			(uint32)root_parameters.size(), root_parameters.data(), 
			(uint32)static_samplers.size(), static_samplers.data(), 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// flag direct indexing enabled/disabled
		if (desc.m_direct_indexing)
		{
			rootSignatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
			rootSignatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
		}

		ID3DBlob* signature;
		ID3DBlob* error;
		HRESULT res = ::D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		res = mpdx_devices[0]->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&dxrootsignature));

		return new_child<dx12_rootsignature, rootsignature>(dxrootsignature, desc, name_to_param_idx);
	}

	ptr<graphics_pipeline> dx12_device::create_graphics_pipeline(rootsignature* rootsig, const graphics_pipeline_desc& desc)
	{
		ID3D12PipelineState* dxpipeline = nullptr;

		// input layout
		D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
		vector< D3D12_INPUT_ELEMENT_DESC> input_elements{};
		for (const graphics_pipeline_desc::pipeline_input_element& element : desc.m_input_elements)
		{
			input_elements.push_back({});
			input_elements.back().AlignedByteOffset = element.m_aligned_byteoffset;
			input_elements.back().Format = translate(element.m_format);
			input_elements.back().InputSlot = element.m_input_slot;
			input_elements.back().InputSlotClass = element.m_is_per_instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			input_elements.back().InstanceDataStepRate = element.m_instance_data_steprate;
			input_elements.back().SemanticIndex = element.m_semantic_idx;
			input_elements.back().SemanticName = element.m_semantic_name.c_str();
		}
		input_layout_desc.pInputElementDescs = input_elements.data();
		input_layout_desc.NumElements = (uint32)input_elements.size();

		// depth stencil
		CD3DX12_DEPTH_STENCIL_DESC depth_stencil_desc(D3D12_DEFAULT);
		depth_stencil_desc.DepthEnable = desc.m_depth_stencil.m_depth_enable;
		depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = translate(desc.m_depth_stencil.m_depth_func);
		depth_stencil_desc.StencilEnable = desc.m_depth_stencil.m_stencil_enable;

		// rasterizer
		CD3DX12_RASTERIZER_DESC rasterizer_desc(D3D12_DEFAULT);
		rasterizer_desc.CullMode = translate(desc.m_rasterizer.m_cullmode);
		rasterizer_desc.FillMode = translate(desc.m_rasterizer.m_fillmode);
		rasterizer_desc.MultisampleEnable = desc.m_rasterizer.m_multisample;
		rasterizer_desc.FrontCounterClockwise = desc.m_rasterizer.m_front_ccw;
		rasterizer_desc.DepthBias = desc.m_rasterizer.m_depth_bias;
		rasterizer_desc.DepthBiasClamp = desc.m_rasterizer.m_depth_bias_clamp;
		rasterizer_desc.SlopeScaledDepthBias = desc.m_rasterizer.m_slope_depth_bias;
		rasterizer_desc.DepthClipEnable = desc.m_rasterizer.m_depth_clip_enable;
		rasterizer_desc.AntialiasedLineEnable = desc.m_rasterizer.m_antialiased_line;
		rasterizer_desc.ForcedSampleCount = desc.m_rasterizer.m_forced_samplecount;
		rasterizer_desc.ConservativeRaster = desc.m_rasterizer.m_conservative ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// blend state
		CD3DX12_BLEND_DESC blend_desc(D3D12_DEFAULT);
		for (size_t i = 0u; i < k_max_render_targets; ++i)
		{
			blend_desc.RenderTarget[i].BlendEnable		= desc.m_blends[i].m_enabled;
			blend_desc.RenderTarget[i].SrcBlend			= translate(desc.m_blends[i].m_src);
			blend_desc.RenderTarget[i].DestBlend		= translate(desc.m_blends[i].m_dest);
			blend_desc.RenderTarget[i].BlendOp			= translate(desc.m_blends[i].m_op);
			blend_desc.RenderTarget[i].SrcBlendAlpha	= translate(desc.m_blends[i].m_srcalpha);
			blend_desc.RenderTarget[i].DestBlendAlpha	= translate(desc.m_blends[i].m_destalpha);
			blend_desc.RenderTarget[i].BlendOpAlpha		= translate(desc.m_blends[i].m_op_alpha);
			blend_desc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // desc.m_blends[i].m_write_mask;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = input_layout_desc;
		pso_desc.pRootSignature = rootsig->get_native<ID3D12RootSignature>();
		pso_desc.VS = CD3DX12_SHADER_BYTECODE(desc.m_vs.data(), desc.m_vs.size());
		pso_desc.PS = CD3DX12_SHADER_BYTECODE(desc.m_ps.data(), desc.m_ps.size());
		pso_desc.RasterizerState = rasterizer_desc;
		pso_desc.BlendState = blend_desc;
		pso_desc.DepthStencilState = depth_stencil_desc;
		pso_desc.SampleMask = desc.m_sample_mask;
		pso_desc.PrimitiveTopologyType = translate(desc.m_prim_type);
		pso_desc;
		pso_desc.DSVFormat = translate(desc.m_format_dsv);
		pso_desc.SampleDesc.Count = desc.m_sample_count;

		// rtvs
		for (size_t i = 0u; i < k_max_render_targets; ++i)
		{
			pso_desc.NumRenderTargets++;
			pso_desc.RTVFormats[i] = translate(desc.m_rtvs[i].m_format);
			pso_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		// create the pipeline
		HRESULT res = mpdx_devices[0]->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&dxpipeline));
		return new_child<dx12_pipeline<e_pipeline_type::graphics>, graphics_pipeline>(dxpipeline, desc);
	}

	ptr<compute_pipeline> dx12_device::create_compute_pipeline(rootsignature* rootsig, const compute_pipeline_desc& desc)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc{};
		pso_desc.CS = CD3DX12_SHADER_BYTECODE(desc.m_cs.data(), desc.m_cs.size());
		pso_desc.pRootSignature = rootsig->get_native<ID3D12RootSignature>();
		pso_desc.CachedPSO;
		pso_desc.Flags;

		ID3D12PipelineState* dxpipeline = nullptr;
		HRESULT res = mpdx_devices[0]->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&dxpipeline));
		return new_child<dx12_pipeline<e_pipeline_type::compute>, compute_pipeline>(dxpipeline, desc);
	}

	void dx12_device::copy_descriptors(const descriptor_range& source, const descriptor_range& dest, const graphics::e_descriptor_heap_type& heap_type)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE source_start{};
		source_start.ptr = (SIZE_T)source.m_start;

		D3D12_CPU_DESCRIPTOR_HANDLE dest_start{};
		dest_start.ptr = (SIZE_T)dest.m_start;

		mpdx_devices[0]->CopyDescriptorsSimple(
			source.m_num_descriptors,
			dest_start,
			source_start,
			translate(heap_type));
	}

	void* dx12_device::get_native()
	{
		return mpdx_devices[0u];
	}

	vector<dx12_device::command_alloc_entry>& dx12_device::get_allocators(const D3D12_COMMAND_LIST_TYPE& type)
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT: return m_direct_allocators;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: return m_compute_allocators;
		case D3D12_COMMAND_LIST_TYPE_COPY: return m_copy_allocators;
		}

		return m_direct_allocators;
	}

	vector<base*>::iterator dx12_device::find_child(base* ptr)
	{
		return std::find(m_children.begin(), m_children.end(), ptr);
	}

	ID3D12CommandAllocator* dx12_device::new_allocator(const D3D12_COMMAND_LIST_TYPE& type)
	{
		vector<command_alloc_entry>& allocators = get_allocators(type);

		for (uint64 i = 0u; i < allocators.size(); ++i)
		{
			// find an already created allocator that was returned
			if (allocators[i].m_allocator != nullptr && allocators[i].m_in_flight == false)
			{
				allocators[i].m_allocator->Reset();
				allocators[i].m_in_flight = true;
				return allocators[i].m_allocator;
			}
		}

		auto create_allocator = [this]() -> ID3D12CommandAllocator*
		{
			ID3D12CommandAllocator* result = dx12helpers::create_command_allocator(mpdx_devices[0u], D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
			result->Reset();
			return result;
		};

		// make sure there's no holes in our vector
		for (uint64 i = 0u; i < allocators.size(); ++i)
		{
			if (allocators[i].m_allocator == nullptr)
			{
				allocators[i].m_allocator = create_allocator();
				allocators[i].m_in_flight = true;
				return allocators[i].m_allocator;
			}
		}

		// create new entry if needed
		command_alloc_entry new_entry{};
		new_entry.m_in_flight = true;
		new_entry.m_allocator = create_allocator();
		allocators.push_back(new_entry);
		return allocators.back().m_allocator;
	}

	void dx12_device::free_allocator(const D3D12_COMMAND_LIST_TYPE& type, ID3D12CommandAllocator* allocator)
	{
		vector<command_alloc_entry>& allocators = get_allocators(type);
		for (uint64 i = 0u; i < allocators.size(); ++i)
		{
			if (allocators[i].m_allocator == allocator)
			{
				allocators[i].m_in_flight = false;
				allocators[i].m_allocator->Reset();
			}
		}
	}
}