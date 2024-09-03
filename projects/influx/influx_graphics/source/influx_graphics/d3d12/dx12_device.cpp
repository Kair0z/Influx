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
#include "influx_graphics/d3d12/dx12_pipeline.h"
#include "influx_graphics/d3d12/dx12_rootsignature.h"

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

#if _DEBUG
		for (size_t i = 0u; i < mpdx_devices.size(); ++i)
		{
			ID3D12InfoQueue* info_queue;
			HRESULT res = mpdx_devices[i]->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (res == S_OK)
			{
				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
					// Workarounds for debug layer issues on hybrid-graphics systems
					D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
					D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
					D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
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

	// get info about physical devices:
	vector<physical_device_info> dx12_device::get_gpu_infos()
	{
		vector<physical_device_info> result_infos{};

		for (size_t i = 0u; i < mpdxgi_adapters.size(); ++i)
		{

		}

		return result_infos;
	}

	const memory_info& dx12_device::get_memory_info() const
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
		auto dxheap = dx12helpers::create_descriptor_heap(mpdx_devices[0u], convert(args.m_type), args.m_capacity, args.m_shader_visible);
		return new dx12_descriptor_heap(args, dxheap, get_descriptor_stride(args.m_type));
	}

	command_allocator* dx12_device::create_graphics_allocator()
	{
		auto dxallocator = dx12helpers::create_command_allocator(mpdx_devices[0u], D3D12_COMMAND_LIST_TYPE_DIRECT);
		return new dx12_command_allocator(dxallocator);
	}

	command_list* dx12_device::create_graphics_command_list(command_allocator* allocator, pipeline* init_state)
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

	resource* dx12_device::create_resource(const tex2D_desc& desc, const heap_desc& heap_desc)
	{
		auto dxresource = dx12helpers::create_tex2d_resource<ID3D12Resource>(mpdx_devices[0u],
			convert(heap_desc.m_type),
			convert(desc.m_format), 
			desc.m_dimensions.x, desc.m_dimensions.y, 
			desc.m_arraysize, desc.m_num_mips,
			desc.m_sample_count, 
			convert(desc.m_flags), 
			convert(desc.m_init_state));

		return new dx12_resource(dxresource, desc);
	}

	resource* dx12_device::create_resource(const buffer_desc& desc, const heap_desc& heap_desc)
	{
		auto dxresource = dx12helpers::create_buffer_resource<ID3D12Resource>(mpdx_devices[0], 
			convert(heap_desc.m_type),
			desc.m_bytesize, 
			convert(desc.m_flags), 
			convert(desc.m_init_state));

		return new dx12_resource(dxresource, desc);
	}

	render_target_view* dx12_device::create_rtv(descriptor_heap* rtv_heap, resource* resource)
	{
		// allocate a new rtv descriptor:
		descriptor_handle cpu_handle = rtv_heap->allocate_cpu();
		return create_rtv(cpu_handle, resource);
	}

	render_target_view* dx12_device::create_rtv(descriptor_handle handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(handle) };

		// create the rtv
		ID3D12Resource* dxresource = resource->get_native<ID3D12Resource>();
		auto dx_rtv = dx12helpers::create_rtv(mpdx_devices[0u], dxresource,
			new_dxdescriptor, convert(resource->get_format()));

		// setup resource info
		resource_info res_info{};
		res_info.m_dimensions = { resource->get_width(), resource->get_height() };

		return new dx12_render_target_view(dx_rtv, res_info);
	}

	depth_stencil_view* dx12_device::create_dsv(descriptor_heap* dsv_heap, resource* resource)
	{
		// allocate a new rtv descriptor:
		descriptor_handle cpu_handle = dsv_heap->allocate_cpu();
		return create_dsv(cpu_handle, resource);
	}

	depth_stencil_view* dx12_device::create_dsv(descriptor_handle handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(handle) };

		// create the dsv
		ID3D12Resource* dxresource = resource->get_native<ID3D12Resource>();
		auto dx_dsv = dx12helpers::create_dsv(mpdx_devices[0u], dxresource,
			new_dxdescriptor, convert(resource->get_format()));

		return new dx12_depth_stencil_view(dx_dsv);
	}

	shader_resource_view* dx12_device::create_srv(descriptor_heap* irv_heap, resource* resource)
	{
		// allocate new srv descriptors
		descriptor_handle cpu_handle = irv_heap->allocate_cpu();
		return create_srv(cpu_handle, {}, resource);
	}

	shader_resource_view* dx12_device::create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { (size_t)(cpu_handle) };
		D3D12_GPU_DESCRIPTOR_HANDLE dxgpu_descriptor = { (size_t)(gpu_handle) };

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = convert(resource->get_format());
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;

		mpdx_devices[0u]->CreateShaderResourceView(
			resource->get_native<ID3D12Resource>(),
			&srv_desc, dxcpu_descriptor);

		return new dx12_shader_resource_view(dxcpu_descriptor, dxgpu_descriptor);
	}

	shader_resource_view* dx12_device::create_buffer_srv(descriptor_heap* srv_heap, resource* resource)
	{
		// allocate new srv descriptors
		descriptor_handle cpu_handle = srv_heap->allocate_cpu();
		return create_buffer_srv(cpu_handle, {}, resource);
	}

	shader_resource_view* dx12_device::create_buffer_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dxcpu_descriptor = { (size_t)(cpu_handle) };
		D3D12_GPU_DESCRIPTOR_HANDLE dxgpu_descriptor = { (size_t)(gpu_handle) };

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.NumElements = resource->get_num_elements();
		srv_desc.Buffer.StructureByteStride = resource->get_bytestride();
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		mpdx_devices[0u]->CreateShaderResourceView(
			resource->get_native<ID3D12Resource>(),
			&srv_desc, dxcpu_descriptor);

		return new dx12_shader_resource_view(dxcpu_descriptor, dxgpu_descriptor);
	}

	sampler_view* dx12_device::create_sampview(descriptor_heap* samp_heap, resource* resource)
	{
		ID3D12DescriptorHeap* heap = samp_heap->get_native<ID3D12DescriptorHeap>();

		// allocate a new rtv descriptor:
		descriptor_handle new_descriptor = samp_heap->allocate_cpu();
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(new_descriptor) };

		return create_sampview(new_descriptor, resource);
	}

	sampler_view* dx12_device::create_sampview(descriptor_handle handle, resource* resource)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE new_dxdescriptor = { (size_t)(handle) };

		// ... no extra code necessary

		return new dx12_sampler_view(new_dxdescriptor);
	}

	rootsignature* dx12_device::create_rootsignature(const rootsignature_desc& desc)
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
			name_to_param_idx[constants.m_common.m_name] = root_parameters.size();

			root_parameters.push_back({});
			root_parameters.back().InitAsConstants(constants.m_num_dwords, constants.m_common.m_shader_register, 
				constants.m_common.m_register_space, convert(constants.m_common.m_visibility));
		}

		// resources
		for (const root_param_resource& resource : desc.m_resources)
		{
			name_to_param_idx[resource.m_common.m_name] = root_parameters.size();

			root_parameters.push_back({});
			switch (resource.m_type)
			{
			case root_param_resource::e_type::srv: 
				root_parameters.back().InitAsShaderResourceView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					convert(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::cbv:
				root_parameters.back().InitAsConstantBufferView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					convert(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::uav:
				root_parameters.back().InitAsUnorderedAccessView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					convert(resource.m_common.m_visibility));
				break;
			}
		}

		// resource tables
		size_t descriptor_table_idx = 0u;
		for (const root_param_resource_table& tables : desc.m_resource_tables)
		{
			name_to_param_idx[tables.m_common.m_name] = root_parameters.size();

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
			root_parameters.back().InitAsDescriptorTable((uint32)ranges.size(), ranges.data(), convert(tables.m_common.m_visibility));
		}

		// samplers
		for (const root_static_sampler& sampler : desc.m_static_samplers)
		{
			static_samplers.push_back({});
			static_samplers.back().Init(
				sampler.m_common.m_shader_register,
				convert(sampler.m_filter),
				convert(sampler.m_wrap_u),
				convert(sampler.m_wrap_v),
				convert(sampler.m_wrap_w),
				sampler.m_mip_lod_bias,
				sampler.m_max_anisotropy,
				convert(sampler.m_comparison_func),
				convert(sampler.m_border_color),
				sampler.m_min_lod,
				sampler.m_max_lod,
				convert(sampler.m_common.m_visibility));
		}

		// initialize the desc, and create the root signature
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

		// flag direct indexing enabled/disabled
		rootSignatureDesc.Desc_1_1.Flags = desc.m_direct_indexing
			? (D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED) : D3D12_ROOT_SIGNATURE_FLAG_NONE;

		rootSignatureDesc.Init_1_1(
			(uint32)root_parameters.size(), root_parameters.data(), 
			(uint32)static_samplers.size(), static_samplers.data(), 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob* signature;
		ID3DBlob* error;
		HRESULT res = ::D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		res = mpdx_devices[0]->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&dxrootsignature));

		return new dx12_rootsignature(dxrootsignature, desc, name_to_param_idx);
	}

	pipeline* dx12_device::create_pipeline(rootsignature* rootsig, const pipeline_desc& desc)
	{
		ID3D12PipelineState* dxpipeline = nullptr;

		// input layout
		D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
		vector< D3D12_INPUT_ELEMENT_DESC> input_elements{};
		for (const pipeline_input_element& element : desc.m_input_elements)
		{
			input_elements.push_back({});
			input_elements.back().AlignedByteOffset = element.m_aligned_byteoffset;
			input_elements.back().Format = convert(element.m_format);
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
		depth_stencil_desc.DepthFunc = convert(desc.m_depth_stencil.m_depth_func);
		depth_stencil_desc.StencilEnable = desc.m_depth_stencil.m_stencil_enable;

		// rasterizer
		CD3DX12_RASTERIZER_DESC rasterizer_desc(D3D12_DEFAULT);
		rasterizer_desc.CullMode = convert(desc.m_rasterizer.m_cullmode);
		rasterizer_desc.FillMode = convert(desc.m_rasterizer.m_fillmode);
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
			blend_desc.RenderTarget[i].SrcBlend			= convert(desc.m_blends[i].m_src);
			blend_desc.RenderTarget[i].DestBlend		= convert(desc.m_blends[i].m_dest);
			blend_desc.RenderTarget[i].BlendOp			= convert(desc.m_blends[i].m_op);
			blend_desc.RenderTarget[i].SrcBlendAlpha	= convert(desc.m_blends[i].m_srcalpha);
			blend_desc.RenderTarget[i].DestBlendAlpha	= convert(desc.m_blends[i].m_destalpha);
			blend_desc.RenderTarget[i].BlendOpAlpha		= convert(desc.m_blends[i].m_op_alpha);
			blend_desc.RenderTarget[i].RenderTargetWriteMask = desc.m_blends[i].m_write_mask;
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
		pso_desc.PrimitiveTopologyType = convert(desc.m_prim_type);
		pso_desc.DSVFormat = convert(desc.m_format_dsv);
		pso_desc.SampleDesc.Count = desc.m_sample_count;

		// rtvs
		for (size_t i = 0u; i < k_max_render_targets; ++i)
		{
			pso_desc.NumRenderTargets++;
			pso_desc.RTVFormats[i] = convert(desc.m_rtvs[i].m_format);
			pso_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		// create the pipeline
		HRESULT res = mpdx_devices[0]->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&dxpipeline));
		return new dx12_pipeline(dxpipeline, desc);
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
			convert(heap_type));
	}

	void* dx12_device::get_native()
	{
		return mpdx_devices[0u];
	}
}