#include "D3D12API.h"

namespace Influx::Graphics
{
	/* D3D12API */
	D3D12API::D3D12API()
	{
		EnableDebugLayer();

		DxgiFactory = D3D12API::CreateDxgiFactory();
		DxgiAdapter = D3D12API::GetAdapter(DxgiFactory, true);
		DxDevice = D3D12API::CreateDevice(DxgiAdapter);

		CreateGlobalDescriptorHeaps();
	}

	D3D12API::~D3D12API()
	{
		SafeRelease(DxgiAdapter);
		SafeRelease(DxDevice);
		SafeRelease(DxgiFactory);

		delete RTVDescriptorHeap;
		delete ResourceDescriptorHeap;
		delete DSVDescriptorheap;
		delete SamplerDescriptorHeap;

		D3D12API::ReportLiveObjects();
	}

	RHICommandQueue* D3D12API::CreateCommandQueue(const ECommandQueueType type) const
	{
		D3D12CommandQueue* result = new D3D12CommandQueue();
		result->eType = type;
		result->DxCommandQueue = CreateDxCommandQueue(DxDevice, Conversion::ToDx12(type));
		result->DxFence = CreateDxFence(DxDevice);
		return result;
	}

	RHISwapChain* D3D12API::CreateSwapChain(HWND windowHandle, RHICommandQueue* commandQueue) const
	{
		D3D12SwapChain* result = new D3D12SwapChain();
		D3D12CommandQueue* dxCommandQueue = (D3D12CommandQueue*)commandQueue;

		RECT rect;
		if (GetWindowRect(windowHandle, &rect))
		{
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			result->Width = (float)width;
			result->Height = (float)height;

			result->DxgiSwapChain = CreateDxgiSwapChain(DxgiFactory, windowHandle, dxCommandQueue->DxCommandQueue, width, height, RHISwapChain::NumBackBuffers);
			result->bIsTearingSupported = D3D12API::CheckDxgiTearingSupport();
			result->CurrentBackBufferIndex = result->DxgiSwapChain->GetCurrentBackBufferIndex();

			// Gather Backbuffers & RTVs
			result->DxRenderTargetDescriptorHeap = CreateDescriptorHeap(DxDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RHISwapChain::NumBackBuffers);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(result->DxRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			D3D12_GPU_DESCRIPTOR_HANDLE rtv_gpu_handle = result->DxRenderTargetDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			
			SIZE_T offsetSize = GetRTVDescriptorSize();
			for (int i = 0; i < RHISwapChain::NumBackBuffers; ++i)
			{
				// Get the buffer resources
				ID3D12Resource* dxBufferResource;
				result->DxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&dxBufferResource));
				result->BackBufferResources[i] = new D3D12Resource(dxBufferResource, ERHIResourceState::Present);
				
				// Create the RenderTargetViews & store
				DxDevice->CreateRenderTargetView(dxBufferResource, nullptr, rtv_cpu_handle);
				D3D12RenderTargetView* d3d12Rtv = new D3D12RenderTargetView();
				d3d12Rtv->DxCPUHandle = rtv_cpu_handle;
				d3d12Rtv->DxGPUHandle = rtv_gpu_handle;
				result->BackBufferRTVs[i] = d3d12Rtv;

				// Offset
				rtv_cpu_handle.ptr = rtv_cpu_handle.ptr + SIZE_T(offsetSize);
				rtv_gpu_handle.ptr = rtv_gpu_handle.ptr + SIZE_T(offsetSize);
			}
		}

		return result;
	}

	RHIVertexBuffer* D3D12API::CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT strideInBytes) const
	{
		D3D12Resource* d3d12Resource = new D3D12Resource();
		D3D12VertexBuffer* d3d12Buffer = new D3D12VertexBuffer(d3d12Resource);
		
		if (initialData != nullptr && initialSizeInBytes > 0)
		{
			D3D12_HEAP_PROPERTIES heapProps{};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;
			
			// Buffer Resource Desc.
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = initialSizeInBytes;
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			// Create Buffer Resource
			DxDevice->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&d3d12Resource->DxResource));

			ID3D12Resource* dxResource = d3d12Resource->DxResource;

			// Copy triangle data to vxBuffer
			UINT8* pDataBegin;
			D3D12_RANGE cpuReadRange;
			cpuReadRange.Begin = 0;
			cpuReadRange.End = 0;

			dxResource->Map(0, &cpuReadRange, reinterpret_cast<void**>(&pDataBegin));
			memcpy(pDataBegin, initialData, initialSizeInBytes);
			dxResource->Unmap(0, nullptr);

			// Initialize VertexBufferView:
			d3d12Buffer->DxVertexBufferView.BufferLocation = dxResource->GetGPUVirtualAddress();
			d3d12Buffer->DxVertexBufferView.StrideInBytes = strideInBytes;
			d3d12Buffer->DxVertexBufferView.SizeInBytes = initialSizeInBytes;
		}

		return d3d12Buffer;
	}

	RHITexture* D3D12API::CreateTexture(const RHITextureDescription& constructionArgs) const
	{
		D3D12Texture* d3d12Texture = new D3D12Texture();
		d3d12Texture->ConstructionDescription = constructionArgs;

		// Create Buffer Resource
		D3D12Resource* d3d12Resource = new D3D12Resource();
		d3d12Texture->Resource = d3d12Resource;
		d3d12Resource->CurrentState = d3d12Resource->PreviousState = constructionArgs.InitialResourceState;

		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = (UINT64)constructionArgs.Width;
		resourceDesc.Height = (UINT64)constructionArgs.Height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = constructionArgs.MipLevels;
		resourceDesc.Format = Conversion::ToDx12(constructionArgs.Format);
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; 

		D3D12_CLEAR_VALUE optimizedClearValue{};
		FLOAT clearColor[4] 
			= { constructionArgs.OptimizedClearValue[0], constructionArgs.OptimizedClearValue[1], 
			constructionArgs.OptimizedClearValue[2], constructionArgs.OptimizedClearValue[3] };

		optimizedClearValue.Format = resourceDesc.Format;
		memcpy(optimizedClearValue.Color, &constructionArgs.OptimizedClearValue, sizeof(optimizedClearValue.Color));

		DxDevice->CreateCommittedResource(&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			Conversion::ToDx12(constructionArgs.InitialResourceState),
			&optimizedClearValue,
			IID_PPV_ARGS(&d3d12Resource->DxResource));

		// Todo: Upload Heap?

		// Create RTV:
		d3d12Texture->RenderTargetView = D3D12API::CreateRenderTargetView(d3d12Texture);

		return d3d12Texture;
	}

	RHIRenderTargetView* D3D12API::CreateRenderTargetView(RHITexture* texture) const
	{
		// Todo: Find first free index & pop it off the list:
		int freeIndex = *RTVDescriptorHeap->FreeIndices.begin();
		RTVDescriptorHeap->FreeIndices.pop_front();

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle = RTVDescriptorHeap->DxDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		cpu_desc_handle.ptr += (GetRTVDescriptorSize() * freeIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle = RTVDescriptorHeap->DxDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		gpu_desc_handle.ptr += (GetRTVDescriptorSize() * freeIndex);

		D3D12RenderTargetView* d3d12RTV = new D3D12RenderTargetView();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = Conversion::ToDx12(texture->GetRHIFormat());
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12Resource* d3d12Resource = (D3D12Resource*)((D3D12Texture*)texture)->Resource;
		DxDevice->CreateRenderTargetView(d3d12Resource->DxResource,
			&rtvDesc, cpu_desc_handle);

		d3d12RTV->DxCPUHandle = cpu_desc_handle;
		d3d12RTV->DxGPUHandle = gpu_desc_handle;

		return d3d12RTV;
	}

	RHIGraphicsPipelineLayout* D3D12API::CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const
	{
		D3D12GraphicsPipelineLayout* graphicsPipelineLayout = new D3D12GraphicsPipelineLayout();
		graphicsPipelineLayout->ConstructionDescription = constructionArgs;

		// Check for Root Signature Feature support...
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(DxDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages...
		D3D12_ROOT_SIGNATURE_FLAGS flags = 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		std::vector<D3D12_ROOT_PARAMETER1> rootSigParams{};
		std::vector<D3D12_STATIC_SAMPLER_DESC> rootSigStaticSamplers{};
		const std::vector<std::shared_ptr<Internal::BaseResourceBinding>>& rhiResourceBindings = constructionArgs.LayoutBindings.ResourceBindings;
		for (const std::shared_ptr<Internal::BaseResourceBinding>& rhiBinding : rhiResourceBindings)
		{
			D3D12_ROOT_PARAMETER1 newDxParam{};
			newDxParam.ParameterType = Conversion::ToDx12(rhiBinding->GetBindingType());
			newDxParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // Todo: Not good!

			switch (rhiBinding->GetBindingType())
			{
			case ERHIResourceBindingType::Constants:
				newDxParam.Constants.Num32BitValues = rhiBinding->GetNum();
				newDxParam.Constants.RegisterSpace = rhiBinding->GetBindingSpace();
				newDxParam.Constants.ShaderRegister = 0; // Todo: ???
				
				break;

			case ERHIResourceBindingType::CBV:
			case ERHIResourceBindingType::SRV:
			case ERHIResourceBindingType::UAV:
				newDxParam.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
				newDxParam.Descriptor.RegisterSpace = rhiBinding->GetBindingSpace();
				newDxParam.Descriptor.ShaderRegister = 0; // Todo: ???
				break;

			default: // Todo: Do we care? Is this possible? // !! Descriptor Tables !!
				break;
			}
			rootSigParams.push_back(newDxParam);
		}

		constexpr static D3D12_ROOT_SIGNATURE_FLAGS DefaultFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Version = featureData.HighestVersion;
		rootSignatureDesc.Desc_1_1.NumParameters = (uint32_t)rootSigParams.size();
		rootSignatureDesc.Desc_1_1.pParameters = rootSigParams.data();
		rootSignatureDesc.Desc_1_1.NumStaticSamplers = (uint32_t)rootSigStaticSamplers.size();
		rootSignatureDesc.Desc_1_1.pStaticSamplers = rootSigStaticSamplers.data();
		rootSignatureDesc.Desc_1_1.Flags = DefaultFlags;

		ID3DBlob* rootSignatureBlob;
		ID3DBlob* errorBlob;
		D3D12API::SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
		HRESULT res = DxDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsPipelineLayout->DxRootSignature));
		if (!SUCCEEDED(res))
		{
			assert(false);
		}

		return graphicsPipelineLayout;
	}

	RHIGraphicsPipeline* D3D12API::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const
	{
		D3D12GraphicsPipeline* d3d12Pipeline = new D3D12GraphicsPipeline();
		D3D12GraphicsPipelineLayout* d3d12Layout = (D3D12GraphicsPipelineLayout*)pipelineLayoutReference;
		d3d12Pipeline->PipelinelayoutReference = d3d12Layout;
		d3d12Pipeline->ConstructionDescription = constructionArgs;

		D3D12GraphicsPipeline::StateStream& stateStream = d3d12Pipeline->PipelineStateStream;

		// Root Signature
		stateStream.RootSignature = d3d12Layout->DxRootSignature;

		// InputLayout
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		stateStream.InputLayout = { inputLayout, _countof(inputLayout) };

		// Primitive Topology Type
		stateStream.PrimitiveTopologyType = Conversion::ToDx12(constructionArgs.PrimitiveTopologyType);

		// RTV Formats
		D3D12_RT_FORMAT_ARRAY rtvFormats{};
		rtvFormats.NumRenderTargets = constructionArgs.RTVFormats.size();
		for (size_t i = 0; i < rtvFormats.NumRenderTargets; ++i)
		{
			rtvFormats.RTFormats[i] = Conversion::ToDx12(constructionArgs.RTVFormats[i]);
		}
		stateStream.RtvFormats = rtvFormats;

		// DSV Single Format
		stateStream.DsvFormat = Conversion::ToDx12(constructionArgs.DSVFormat);

		// Depth Stencil
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = constructionArgs.bDepthEnabled;
		depthStencilDesc.StencilEnable = constructionArgs.bStencilEnabled;
		stateStream.DepthStencil = depthStencilDesc;

		// Shaders:
		for (RHIShader* shader : { constructionArgs.PixelShader, constructionArgs.VertexShader })
		{
			D3D12Shader* d3d12Shader = (D3D12Shader*)shader;
			ID3DBlob* dxShaderBlob = d3d12Shader->DxShaderBlob;
			D3D12_SHADER_BYTECODE shaderByteCode{};
			shaderByteCode.BytecodeLength = dxShaderBlob->GetBufferSize();
			shaderByteCode.pShaderBytecode = dxShaderBlob->GetBufferPointer();

			switch (shader->GetType())
			{
			default:
			case ERHIShaderType::PixelShader:
				stateStream.PixelShader = shaderByteCode;
				break;

			case ERHIShaderType::VertexShader:
				stateStream.VertexShader = shaderByteCode;
				break;
			}
		}

		// Rasterizer
		D3D12_RASTERIZER_DESC rasterDesc{};
		rasterDesc.CullMode = Conversion::ToDx12(constructionArgs.RasterCullMode);
		rasterDesc.FillMode = Conversion::ToDx12(constructionArgs.RasterFillMode);
		rasterDesc.ConservativeRaster = (constructionArgs.bConservativeRaster) ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		rasterDesc.DepthBias = constructionArgs.RasterDepthBias;
		rasterDesc.DepthBiasClamp = constructionArgs.RasterMaxDepthBias;
		rasterDesc.DepthClipEnable = constructionArgs.bRasterDepthClipEnable;
		rasterDesc.AntialiasedLineEnable = constructionArgs.bAntialiasedLineEnable;
		stateStream.Rasterizer = rasterDesc;

		const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{ sizeof(D3D12GraphicsPipeline::StateStream), &d3d12Pipeline->PipelineStateStream };
		HRESULT res = DxDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&d3d12Pipeline->DxPipelineState));
		if (FAILED(res))
		{
			assert(false);
		}

		return d3d12Pipeline;
	}

	RHIShader* D3D12API::CreateRHIShader(const std::vector<uint8_t>& fromCompiledData) const
	{
		return nullptr;
	}

	RHIShader* D3D12API::CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const std::string& target) const
	{
		D3D12Shader* d3d12Shader = new D3D12Shader();

		ParseShaderTargetString(target, d3d12Shader->Type, d3d12Shader->ShaderModel);
		
		ID3DBlob* pError = nullptr;
		const D3D_SHADER_MACRO* pDefines = nullptr;
		ID3DInclude* pInclude = nullptr;
		UINT flags1 = 0;
		UINT flags2 = 0;

		D3DCompileFromFile(fromFilePath.c_str(),
			pDefines, pInclude, entryPoint.c_str(), target.c_str(), 
			flags1, flags2, &d3d12Shader->DxShaderBlob, &pError);

		return d3d12Shader;
	}

	RHIShader* D3D12API::CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const ERHIShaderType shaderType, const ERHIShaderModel shaderModel) const
	{
		D3D12Shader* d3d12Shader = new D3D12Shader();
		d3d12Shader->Type = shaderType;
		d3d12Shader->ShaderModel = shaderModel;

		const std::string& targetString = MakeShaderTargetString(shaderType, shaderModel);

		ID3DBlob* pError = nullptr;
		const D3D_SHADER_MACRO* pDefines = nullptr;
		ID3DInclude* pInclude = nullptr;
		UINT flags1 = 0;
		UINT flags2 = 0;

		D3DCompileFromFile(fromFilePath.c_str(),
			pDefines, pInclude, entryPoint.c_str(), targetString.c_str(),
			flags1, flags2, &d3d12Shader->DxShaderBlob, &pError);

		return d3d12Shader;
	}

	ID3D12Device2* D3D12API::GetDxDevice() const
	{
		return DxDevice;
	}
	IDXGIAdapter4* D3D12API::GetDxgiAdapter() const
	{
		return DxgiAdapter;
	}
	IDXGIFactory4* D3D12API::GetDxgiFactory() const
	{
		return DxgiFactory;
	}

	const size_t D3D12API::GetRTVDescriptorSize() const
	{
		return CachedRtvDescriptorSize;
	}
	const size_t D3D12API::GetDSVDescriptorSize() const
	{
		return CachedDsvDescriptorSize;
	}
	const size_t D3D12API::GetResourceDescriptorSize() const
	{
		return CachedResourceDescriptorSize;
	}
	const size_t D3D12API::GetSamplerDescriptorSize() const
	{
		return CachedSamplerDescriptorSize;
	}

	void D3D12API::CreateGlobalDescriptorHeaps()
	{
		// Cache DescriptorSizes
		CachedDsvDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CachedResourceDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CachedSamplerDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		CachedRtvDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Create Global Descriptor Heaps:
		RTVDescriptorHeap = new D3D12DescriptorHeap(ERHIDescriptorType::RTV, 64);
		DSVDescriptorheap = new D3D12DescriptorHeap(ERHIDescriptorType::DSV, 64);
		SamplerDescriptorHeap = new D3D12DescriptorHeap(ERHIDescriptorType::Sampler, 16);
		ResourceDescriptorHeap = new D3D12DescriptorHeap(ERHIDescriptorType::Resource, 64);

		for (D3D12DescriptorHeap* heap : { RTVDescriptorHeap, SamplerDescriptorHeap, DSVDescriptorheap, ResourceDescriptorHeap})
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NodeMask = 1;
			heapDesc.NumDescriptors = heap->MaxNumDescriptors;
			heapDesc.Type = Conversion::ToDx12(heap->Type);

			// Clear & create a Freelist:
			heap->FreeIndices.clear();
			for (UINT64 i = 0; i < heap->MaxNumDescriptors; ++i)
			{
				heap->FreeIndices.push_back(i);
			}

			DxDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap->DxDescriptorHeap));
		}
	}


	/* Dx12 Statics: */
#pragma region D3D12Statics
	IDXGIFactory4* D3D12API::CreateDxgiFactory()
	{
		/* Create Factory... */
		IDXGIFactory4* dxgiFactory;
		UINT flags = 0;
#ifdef _DEBUG
		flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		/* TODO: Throw On Fail... */
		CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory));

		return dxgiFactory;
	}

	IDXGIAdapter4* D3D12API::GetAdapter(IDXGIFactory4* dxgiFactory, bool useWarp)
	{
		/* Get sufficient Adapter ...*/
		IDXGIAdapter1* dxgiAdapter1{};
		IDXGIAdapter4* dxgiAdapter4{};
		if (useWarp)
		{
			dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
			dxgiAdapter4 = (IDXGIAdapter4*)dxgiAdapter1;
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				// Check to see if the adapter can create a D3D12 device without actually 
				// creating it. The adapter with the largest dedicated video memory
				// is favored.
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1,
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					dxgiAdapter4 = (IDXGIAdapter4*)dxgiAdapter1;
				}
			}
		}

		return dxgiAdapter4;
	}

	ID3D12Device2* D3D12API::CreateDevice(IDXGIAdapter4* pAdapter)
	{
		ID3D12Device2* d3d12Device2{};
		D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));

#ifdef _DEBUG
		ID3D12InfoQueue1* pInfoQueue = nullptr;
		d3d12Device2->QueryInterface(&pInfoQueue);
		if (pInfoQueue != nullptr)
		{
			/* TODO: Why does this crash? */
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			pInfoQueue->PushStorageFilter(&NewFilter);

			pInfoQueue->Release();
		}
#endif
		return d3d12Device2;
	}
	
	ID3D12CommandQueue* D3D12API::CreateDxCommandQueue(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type)
	{
		ID3D12CommandQueue* d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue));

		return d3d12CommandQueue;
	}

	bool D3D12API::CheckDxgiTearingSupport()
	{
		bool allowTearing = false;

		// Rather than create the DXGI 1.5 factory interface directly, we create the
		// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
		// graphics debugging tools which will not support the 1.5 factory interface 
		// until a future update.
		IDXGIFactory4* factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			IDXGIFactory5* factory5;
			if (factory5 = (IDXGIFactory5*)factory4)
			{
				if (FAILED(factory5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING,
					&allowTearing, sizeof(allowTearing))))
				{
					allowTearing = false;
				}
			}
		}

		return allowTearing;
	}

	IDXGISwapChain4* D3D12API::CreateDxgiSwapChain(IDXGIFactory4* dxgiFactory, HWND hWnd, ID3D12CommandQueue* pCommandQueue, UINT32 w, UINT32 h, UINT32 bufferCount)
	{
		IDXGISwapChain4* dxgiSwapChain4;
		UINT flags = 0;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = w;
		desc.Height = h;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Stereo = false;
		desc.SampleDesc = { 1, 0 };
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = bufferCount;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Flags = CheckDxgiTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		IDXGISwapChain1* swapChain1;
		dxgiFactory->CreateSwapChainForHwnd(pCommandQueue, hWnd, &desc, nullptr, nullptr, &swapChain1);

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		dxgiSwapChain4 = (IDXGISwapChain4*)swapChain1;
		return dxgiSwapChain4;
	}

	ID3D12DescriptorHeap* D3D12API::CreateDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors)
	{
		ID3D12DescriptorHeap* descHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap));
		return descHeap;
	}

	ID3D12CommandAllocator* D3D12API::CreateCommandAllocator(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type)
	{
		ID3D12CommandAllocator* cmdAllocator;
		pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&cmdAllocator));
		return cmdAllocator;
	}

	ID3D12GraphicsCommandList* D3D12API::CreateCommandList(ID3D12Device2* pDevice, ID3D12CommandAllocator* cmdAllocator, D3D12_COMMAND_LIST_TYPE type)
	{
		ID3D12GraphicsCommandList* commandList;
		pDevice->CreateCommandList(0, type, cmdAllocator, nullptr, IID_PPV_ARGS(&commandList));
		commandList->Close();
		commandList->Reset(cmdAllocator, nullptr);
		return commandList;
	}

	ID3D12Fence* D3D12API::CreateDxFence(ID3D12Device2* pDevice)
	{
		ID3D12Fence* fence;
		pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

		return fence;
	}

	UINT64 D3D12API::Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue)
	{
		uint64_t fenceValueForSignal = ++fenceValue;
		commandQueue->Signal(fence, fenceValueForSignal);

		return fenceValueForSignal;
	}

	void D3D12API::WaitForFenceValue(ID3D12Fence* fence, UINT64 fenceValue, HANDLE fenceEvent, float durationInMs)
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(durationInMs));
		}
	}

	void D3D12API::FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue, HANDLE fenceEvent)
	{
		uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent, FLT_MAX);
	}

	void D3D12API::SerializeVersionedRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION maxVersion, ID3DBlob** ppOutBlob, ID3DBlob** ppErrorBlob) noexcept
	{
		if (ppErrorBlob != nullptr)
		{
			// Clear Error blob
			*ppErrorBlob = nullptr;
		}
		
		switch (maxVersion)
		{
		case D3D_ROOT_SIGNATURE_VERSION_1_1:
			D3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppOutBlob, ppErrorBlob);
			break;

		case D3D_ROOT_SIGNATURE_VERSION_1_0:
			switch (pRootSignatureDesc->Version)
			{
			case D3D_ROOT_SIGNATURE_VERSION_1_0:
				// If Max version is 1_0 but the rootSignature is also 1_0, there's no problem and we can just call the D3D12 provided function:
				D3D12SerializeRootSignature(&pRootSignatureDesc->Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppOutBlob, ppErrorBlob);
				break;

			case D3D_ROOT_SIGNATURE_VERSION_1_1:
				// Else, we're gonna have to convert...
				assert(false); // Todo: But that's a nice ol' todo ;)
				break;
			}
			break;
		}
	}

	void D3D12API::ReportLiveObjects()
	{
		IDXGIDebug* dxgiControler;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiControler));
		dxgiControler->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}

	void D3D12API::EnableDebugLayer()
	{
#if defined(_DEBUG)
		// Always enable the debug layer before doing anything DX12 related
		// so all possible errors generated while creating DX12 objects
		// are caught by the debug layer.
		ID3D12Debug* debugInterface;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
		debugInterface->EnableDebugLayer();
#endif
	}
#pragma endregion

	/* D3D12CommandQueue */
	RHICommandList* D3D12CommandQueue::SetupNewCommandList(GraphicsAPI* api)
	{
		ID3D12CommandAllocator* dxCmdAllocator;
		D3D12CommandList* returnList;

		D3D12API* dxApi = (D3D12API*)api;

		/* Get a Command Allocator */
		/* The allocator can be reused as long as it's not 'in-flight' on the CmdQueue */
		if (!CommandAllocatorQueue.empty() && IsFenceComplete(CommandAllocatorQueue.front().FenceValue))
		{
			/* The last 'launched' allocator is in front and its fence value has been reached */
			dxCmdAllocator = CommandAllocatorQueue.front().Allocator;
			CommandAllocatorQueue.pop();

			dxCmdAllocator->Reset();
		}
		else
		{
			/* No available allocator */
			dxCmdAllocator = D3D12API::CreateCommandAllocator(dxApi->GetDxDevice(), Conversion::ToDx12(eType));
		}

		/* Get a Command List */
		if (!CommandListQueue.empty())
		{
			/* Pop off the front list */
			returnList = CommandListQueue.front();
			CommandListQueue.pop();

			((ID3D12GraphicsCommandList*)returnList->GetDxCommandList())->Reset(dxCmdAllocator, nullptr);
		}
		else
		{
			returnList = new D3D12CommandList();
			returnList->DxCommandList = D3D12API::CreateCommandList(dxApi->GetDxDevice(), dxCmdAllocator, Conversion::ToDx12(eType));
		}

		/* Set private data so we can query the allocator pointer later from the commandlist :) */
		returnList->GetDxCommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), dxCmdAllocator);

		return returnList;
	}

	void D3D12CommandQueue::ExecuteCommmandList(RHICommandList* commandList)
	{
		D3D12CommandList* d3d12CmdList = (D3D12CommandList*)commandList;
		ID3D12GraphicsCommandList* dxCmdList = d3d12CmdList->DxCommandList;

		dxCmdList->Close();

		/* Get the private held Command Allocator */
		ID3D12CommandAllocator* cmdAllocator;
		UINT dataSize = sizeof(cmdAllocator);
		dxCmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &cmdAllocator);

		/* Execute */
		ID3D12CommandList* const ppCommandLists[] = { dxCmdList };
		DxCommandQueue->ExecuteCommandLists(1, ppCommandLists);

		/* Signal() returns the value we will be waiting for, when the GPU is finished executing this cmdlist,
			the Fence-event will trigger... */
		UINT64 fenceValue = D3D12API::Signal(DxCommandQueue, DxFence, CurrentFenceValue);

		/* Update queues */
		CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, cmdAllocator });
		CommandListQueue.push(d3d12CmdList);

		/* Release the temporary retrieved allocator pointer */
		D3D12API::SafeRelease(cmdAllocator);
	}

	void D3D12CommandQueue::Flush()
	{
		D3D12API::FlushCommandQueue(DxCommandQueue, DxFence, CurrentFenceValue, FenceEventHandle);
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		Flush();

		while (!CommandAllocatorQueue.empty())
		{
			D3D12API::SafeRelease(CommandAllocatorQueue.front().Allocator);
			CommandAllocatorQueue.pop();
		}

		while (!CommandListQueue.empty())
		{
			D3D12API::SafeRelease(CommandListQueue.front()->DxCommandList);
			CommandListQueue.pop();
		}

		D3D12API::SafeRelease(DxCommandQueue);
		D3D12API::SafeRelease(DxFence);
	}

	bool D3D12CommandQueue::IsFenceComplete(UINT64 completeValue) const
	{
		return DxFence->GetCompletedValue() >= completeValue;
	}

	/* D3D12CommandList */
	void D3D12CommandList::TransitionResource(RHIResource* resource, const ERHIResourceState newState)
	{
		if (resource->GetCurrentState() == newState) return;

		D3D12Resource* d3d12Resource = (D3D12Resource*)resource;
		ID3D12Resource* dxResource = d3d12Resource->GetDxResource();

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = dxResource;
		barrier.Transition.StateBefore = Conversion::ToDx12(resource->GetCurrentState());
		barrier.Transition.StateAfter = Conversion::ToDx12(newState);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		
		resource->Transition(newState);
		DxCommandList->ResourceBarrier(1, &barrier);
	}

	void D3D12CommandList::ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue)
	{
		D3D12RenderTargetView* d3d12RTV = (D3D12RenderTargetView*)renderTargetView;

		const FLOAT color[4] = { clearValue[0], clearValue[1], clearValue[2], clearValue[3] };
		DxCommandList->ClearRenderTargetView(d3d12RTV->DxCPUHandle, color, 0, nullptr);
	}

	void D3D12CommandList::BindScissorRect(const RHIScissorRect& scissorRect)
	{
		D3D12_RECT dxRect{};
		dxRect.left = scissorRect.Left;
		dxRect.top = scissorRect.Bottom;
		dxRect.right = scissorRect.Width;
		dxRect.bottom = scissorRect.Height;
		DxCommandList->RSSetScissorRects(1, &dxRect);
	}

	void D3D12CommandList::BindViewports(const RHIViewport& viewport)
	{
		D3D12_VIEWPORT dxViewport{};
		dxViewport.TopLeftY = viewport.Bottom;
		dxViewport.TopLeftX = viewport.Left;
		dxViewport.Width = viewport.Width;
		dxViewport.Height = viewport.Height;
		dxViewport.MaxDepth = 1.0f;
		dxViewport.MinDepth = 0.0f;

		DxCommandList->RSSetViewports(1, &dxViewport);
	}

	void D3D12CommandList::BindVertexBuffer(RHIVertexBuffer* vertexBuffer)
	{
		D3D12VertexBuffer* d3d12VertexBuffer = (D3D12VertexBuffer*)vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vtBufferView = d3d12VertexBuffer->GetDxVertexBufferView();
		DxCommandList->IASetVertexBuffers(0, 1, &vtBufferView);
	}

	void D3D12CommandList::SetPrimitiveTopology(ERHIPrimitiveTopology topology)
	{
		DxCommandList->IASetPrimitiveTopology(Conversion::ToDx12(topology));
	}

	void D3D12CommandList::CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition)
	{
		if (forceTransition)
		{
			TransitionResource(source, ERHIResourceState::CopySource);
			TransitionResource(dest, ERHIResourceState::CopyDest);
		}

		DxCommandList->CopyResource(((D3D12Resource*)dest)->GetDxResource(), ((D3D12Resource*)source)->GetDxResource());
		
		// Force transition back
		if (forceTransition)
		{
			TransitionResource(source, source->GetPreviousState());
			TransitionResource(dest, dest->GetPreviousState());
		}
	}

	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, bool forceTransition)
	{
		ClearTextureAsRTV(texture, texture->GetOptimizedClearValue(), forceTransition);
	}

	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition)
	{
		if (forceTransition)
		{
			TransitionResource(texture->GetRHIResource(), ERHIResourceState::RenderTarget);
		}

		ClearRTV(texture->GetRenderTargetView(), clearValue);

		if (forceTransition)
		{
			TransitionResource(texture->GetRHIResource(), texture->GetRHIResource()->GetPreviousState());
		}
	}

	void D3D12CommandList::BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout)
	{
		D3D12GraphicsPipelineLayout* d3d12Layout = (D3D12GraphicsPipelineLayout*)pipelineLayout;
		DxCommandList->SetGraphicsRootSignature(d3d12Layout->GetDxRootSignature());
	}

	void D3D12CommandList::BindPipelineState(RHIGraphicsPipeline* pipeline)
	{
		D3D12GraphicsPipeline* d3d12Pipeline = (D3D12GraphicsPipeline*)pipeline;
		DxCommandList->SetPipelineState(d3d12Pipeline->GetDxPipelineState());
	}

	void D3D12CommandList::BindRenderTarget(RHIRenderTargetView* renderTargetView)
	{
		D3D12RenderTargetView* d3d12Rtv = (D3D12RenderTargetView*)renderTargetView;
		DxCommandList->OMSetRenderTargets(1, &d3d12Rtv->DxCPUHandle, true, nullptr); // Todo DSV?
	}

	void D3D12CommandList::DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation)
	{
		DxCommandList->DrawInstanced(numVerticesPerInstance, numInstances, startVertexLocation, startInstanceLocation);
	}

	/* D3D12Resource */
	D3D12Resource::D3D12Resource() : D3D12Resource(nullptr, ERHIResourceState::Invalid) {}

	D3D12Resource::D3D12Resource(ID3D12Resource* dxResource, ERHIResourceState initialState)
	{
		PreviousState = CurrentState = initialState;
		DxResource = dxResource;
	}

	D3D12Resource::~D3D12Resource()
	{
		D3D12API::SafeRelease(DxResource);
	}

	ID3D12Resource* D3D12Resource::GetDxResource() const
	{
		return DxResource;
	}

	/* D3D12SwapChain */
	void D3D12SwapChain::Present(bool VSync)
	{
		UINT syncIntv = VSync ? 1 : 0;
		UINT flags = bIsTearingSupported && !VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		DxgiSwapChain->Present(syncIntv, 0);

		CurrentBackBufferIndex = DxgiSwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12SwapChain::Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY)
	{
		commandQueue->Flush();
	}

	D3D12SwapChain::~D3D12SwapChain()
	{
		D3D12API::SafeRelease(DxgiSwapChain);
		D3D12API::SafeRelease(DxRenderTargetDescriptorHeap);
	}

	D3D12VertexBuffer::D3D12VertexBuffer(D3D12Resource* gpuResource)
	{
		GpuResource = gpuResource;
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
		
	}

	D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetDxVertexBufferView() const
	{
		return DxVertexBufferView;
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(const ERHIDescriptorType type, UINT64 maxDescriptorNum)
		: Type{type}
		, MaxNumDescriptors{maxDescriptorNum}
	{
		
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		D3D12API::SafeRelease(DxDescriptorHeap);
	}

	ID3D12RootSignature* D3D12GraphicsPipelineLayout::GetDxRootSignature() const
	{
		return DxRootSignature;
	}

	D3D12GraphicsPipelineLayout::~D3D12GraphicsPipelineLayout()
	{
		D3D12API::SafeRelease(DxRootSignature);
	}

	D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
	{
		D3D12API::SafeRelease(DxPipelineState);
	}

	ID3D12PipelineState* D3D12GraphicsPipeline::GetDxPipelineState() const
	{
		return DxPipelineState;
	}

	D3D12Shader::~D3D12Shader()
	{
		D3D12API::SafeRelease(DxShaderBlob);
	}
}