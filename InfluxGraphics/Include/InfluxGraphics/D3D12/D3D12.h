#pragma once

#ifndef __GR_D3D12_H_
#define __GR_D3D12_H_

#include "InfluxGraphics/Types.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Influx::Graphics::D3D12
{
	using DevicePtr = ID3D12Device2*;

	namespace HelperStructs
	{
		struct CommittedResourceDesc final
		{
		private:
			CommittedResourceDesc() = default;
			D3D12_HEAP_PROPERTIES m_heapProperties;
			D3D12_HEAP_FLAGS m_heapFlags;
			D3D12_RESOURCE_DESC m_resourceDesc;
			D3D12_CLEAR_VALUE m_optimizedClearValue;

		public:
			static CommittedResourceDesc AsTexture(const DXGI_FORMAT format, uint64_t width, uint64_t height, uint16_t numMipLevels)
			{
				CommittedResourceDesc desc{};

				desc.m_heapProperties = GetDefaultHeapProperties();
				desc.m_heapFlags = D3D12_HEAP_FLAG_NONE;
				desc.m_resourceDesc = GetTextureDesc(format, width, height, numMipLevels);
				desc.m_optimizedClearValue = GetOptimizedClearValue(format);

				return desc;
			}

			static D3D12_HEAP_PROPERTIES GetDefaultHeapProperties()
			{
				D3D12_HEAP_PROPERTIES defaultProperties;
				defaultProperties.Type					= D3D12_HEAP_TYPE_DEFAULT;
				defaultProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				defaultProperties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
				defaultProperties.CreationNodeMask		= 0;
				defaultProperties.VisibleNodeMask		= 0;
				return defaultProperties;
			}

			static D3D12_CLEAR_VALUE GetOptimizedClearValue(const DXGI_FORMAT format)
			{
				D3D12_CLEAR_VALUE optimizedClearValue{};
				optimizedClearValue.Color[0] = 0.0f;
				optimizedClearValue.Color[1] = 0.0f;
				optimizedClearValue.Color[2] = 0.0f;
				optimizedClearValue.Color[3] = 1.0f;

				D3D12_DEPTH_STENCIL_VALUE dsValue{};
				dsValue.Depth = 0.0f;
				dsValue.Stencil = 0u;

				optimizedClearValue.DepthStencil = dsValue;
				optimizedClearValue.Format = format;
				return optimizedClearValue;
			}

			static D3D12_RESOURCE_DESC GetTextureDesc(const DXGI_FORMAT format, uint64_t width, uint64_t height, uint16_t numMipLevels)
			{
				D3D12_RESOURCE_DESC textureDesc{};

				textureDesc.Format				= format;
				textureDesc.Width				= (uint32_t)width;
				textureDesc.Height				= (uint32_t)height;
				textureDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				textureDesc.DepthOrArraySize	= 1;
				textureDesc.MipLevels			= numMipLevels;
				textureDesc.SampleDesc.Count	= 1;
				textureDesc.SampleDesc.Quality	= 0;
				textureDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				textureDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
				textureDesc.Alignment			= 0;

				return textureDesc;
			}

			D3D12_HEAP_PROPERTIES GetHeapProperties() const
			{
				return m_heapProperties;
			}

			D3D12_HEAP_FLAGS GetHeapFlags() const
			{
				return m_heapFlags;
			}

			D3D12_RESOURCE_DESC GetResourceDesc() const
			{
				return m_resourceDesc;
			}

			D3D12_CLEAR_VALUE GetOptimizedClearValue() const
			{
				return m_optimizedClearValue;
			}

			bool IsValid()
			{
				// Todo...
				return true;
			}
		};

		struct RootSignatureDesc final
		{
			RootSignatureDesc() = default;

			constexpr static D3D12_ROOT_SIGNATURE_FLAGS k_defaultFlags =
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			Vector<D3D12_ROOT_PARAMETER1>		RootParameters1{};
			Vector<D3D12_ROOT_PARAMETER>		RootParameters{};
			Vector<D3D12_STATIC_SAMPLER_DESC>	StaticSamplers{};
			D3D12_ROOT_SIGNATURE_FLAGS			Flags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };
			D3D12_FEATURE_DATA_ROOT_SIGNATURE	FeatureData;
			D3D_ROOT_SIGNATURE_VERSION			MaxVersion	= D3D_ROOT_SIGNATURE_VERSION_1_1;

		public:
			void AddConstants(uint32 numConstants, D3D12_SHADER_VISIBILITY visibility, uint32 shaderRegister, uint32 space = 0u)
			{
				D3D12_ROOT_PARAMETER param{};
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; // Float constants
				param.ShaderVisibility = visibility;
				param.Constants.Num32BitValues = numConstants;
				param.Constants.ShaderRegister = shaderRegister;
				param.Constants.RegisterSpace = space;
				
				D3D12_ROOT_PARAMETER1 param1{};
				param1.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; // Float constants
				param1.ShaderVisibility = visibility;
				param1.Constants.Num32BitValues = numConstants;
				param1.Constants.ShaderRegister = shaderRegister;
				param1.Constants.RegisterSpace = space;
				
				RootParameters.push_back(param);
				RootParameters1.push_back(param1);
			}

			void AddDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
			{
				D3D12_ROOT_PARAMETER param{};
				param.ParameterType = type;
				param.ShaderVisibility = visibility;
				param.Descriptor.ShaderRegister = shaderRegister;
				param.Descriptor.RegisterSpace = space;
				// param.Descriptor.Flags;

				D3D12_ROOT_PARAMETER1 param1{};
				param1.ParameterType = type;
				param1.ShaderVisibility = visibility;
				param1.Descriptor.ShaderRegister = shaderRegister;
				param1.Descriptor.RegisterSpace = space;
				param1.Descriptor.Flags = flags;

				RootParameters.push_back(param);
				RootParameters1.push_back(param1);
			}

			void AddRootDescriptorTable(D3D12_SHADER_VISIBILITY visibility, const D3D12_DESCRIPTOR_RANGE1* ranges, uint32_t numRanges)
			{
				D3D12_ROOT_PARAMETER param{};
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				param.ShaderVisibility = visibility;
				param.DescriptorTable.NumDescriptorRanges = numRanges;
				// param.DescriptorTable.pDescriptorRanges = ranges;

				D3D12_ROOT_PARAMETER1 param1{};
				param1.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				param1.ShaderVisibility = visibility;
				param1.DescriptorTable.NumDescriptorRanges = numRanges;
				param1.DescriptorTable.pDescriptorRanges = ranges;

				RootParameters.push_back(param);
				RootParameters1.push_back(param1);
			}

			void AddConstantBufferView(D3D12_SHADER_VISIBILITY visibility, uint32 shaderRegister, uint32 space = 0u, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
			{
				AddDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shaderRegister, space, flags);
			}

			void AddShaderResourceView(D3D12_SHADER_VISIBILITY visibility, uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
			{
				AddDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shaderRegister, space, flags);
			}

			void AddUnorderedAccessView(D3D12_SHADER_VISIBILITY visibility, uint32 shaderRegister, uint32 space = 0u, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
			{
				AddDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shaderRegister, space, flags);
			}
		};

		struct GraphicsPipelineStateDesc final
		{
			Vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
			D3D12_SHADER_BYTECODE VertexShaderBytecode;
			D3D12_SHADER_BYTECODE PixelShaderByteCode;
			D3D12_RASTERIZER_DESC RasterizerState;
			D3D12_BLEND_DESC BlendState;
			D3D12_DEPTH_STENCIL_DESC DepthStencilState;
			uint8 SampleMask;
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
			Vector<DXGI_FORMAT> RenderTargetFormats; // Implies NumRenderTargets...
			DXGI_SAMPLE_DESC SampleDesc;

			void AddInputElement(const char* semanticName, uint8 semanticIndex, DXGI_FORMAT format, uint8 inputSlot, uint8 alignedByteOffset, D3D12_INPUT_CLASSIFICATION inputSlotClass, uint8 instanceDataStepRate)
			{
				D3D12_INPUT_ELEMENT_DESC newElement{};
				newElement.SemanticName = semanticName;
				newElement.SemanticIndex = semanticIndex;
				newElement.Format = format;
				newElement.InputSlot = inputSlot;
				newElement.AlignedByteOffset = alignedByteOffset;
				newElement.InputSlotClass = inputSlotClass;
				newElement.InstanceDataStepRate = instanceDataStepRate;
				InputElements.push_back(newElement);
			}
		};
	}

	template <typename _T>
	inline void SafeRelease(_T*& object)
	{
		if (object == nullptr)
		{
			return;
		}

		object->Release();
		object = nullptr;
	}

	inline IDXGIFactory4* CreateDxgiFactory4()
	{
		/* Create Factory... */
		IDXGIFactory4* dxgiFactory;
		uint8 flags = 0;

#ifdef _DEBUG
		flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		/* TODO: Throw On Fail... */
		CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory));

		return dxgiFactory;
	}

	inline IDXGIAdapter4* GetDxgiAdapter4(IDXGIFactory4* dxgiFactory4, bool useWarp)
	{
		/* Get sufficient Adapter ...*/
		IDXGIAdapter1* dxgiAdapter1{};
		IDXGIAdapter4* dxgiAdapter4{};
		if (useWarp)
		{
			dxgiFactory4->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
			dxgiAdapter4 = (IDXGIAdapter4*)dxgiAdapter1;
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory4->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
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

	inline DevicePtr CreateDxDevice2(IDXGIAdapter4* adapter4)
	{
		DevicePtr d3d12Device2{};
		D3D12CreateDevice(adapter4, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));

#ifdef _DEBUG
		ID3D12InfoQueue* pInfoQueue = nullptr;
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

	inline ID3D12CommandQueue* CreateDxCommandQueue(DevicePtr pDevice, D3D12_COMMAND_LIST_TYPE type)
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

	inline bool CheckDxgiTearingSupport()
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

	/*
	* ID3D12Device::CreateSwapChainForHwnd
	* ID3D12Device::MakeWindowAssociation
	*/
	inline IDXGISwapChain4* CreateDxgiSwapChain(
		IDXGIFactory4* dxgiFactory, ::HWND hWnd, ID3D12CommandQueue* pCommandQueue, 
		uint32 w, uint32 h, uint8 numBuffers, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		IDXGISwapChain4* dxgiSwapChain4;
		UINT flags = 0;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width			= w;
		desc.Height			= h;
		desc.Format			= format;
		desc.Stereo			= false;
		desc.SampleDesc		= { 1, 0 };
		desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount	= numBuffers;
		desc.Scaling		= DXGI_SCALING_STRETCH;
		desc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode		= DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Flags			= CheckDxgiTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		IDXGISwapChain1* swapChain1;
		dxgiFactory->CreateSwapChainForHwnd(pCommandQueue, hWnd, &desc, nullptr, nullptr, &swapChain1);

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		dxgiSwapChain4 = (IDXGISwapChain4*)swapChain1;
		return dxgiSwapChain4;
	}

	/* ID3D12Device::CreateDescriptorHeap */
	inline ID3D12DescriptorHeap* CreateDxDescriptorHeap(DevicePtr pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
		ID3D12DescriptorHeap* descHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = flags;

		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap));
		return descHeap;
	}

	/* ID3D12Device::CreateDxCommandAllocator */
	inline ID3D12CommandAllocator* CreateDxCommandAllocator(DevicePtr pDevice, D3D12_COMMAND_LIST_TYPE type)
	{
		ID3D12CommandAllocator* cmdAllocator;
		pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&cmdAllocator));
		return cmdAllocator;
	}

	/* ID3D12Device::CreateCommandList */
	inline ID3D12GraphicsCommandList* CreateDxCommandList(DevicePtr pDevice, ID3D12CommandAllocator* cmdAllocator, D3D12_COMMAND_LIST_TYPE type)
	{
		ID3D12GraphicsCommandList* commandList;
		pDevice->CreateCommandList(0, type, cmdAllocator, nullptr, IID_PPV_ARGS(&commandList));

		commandList->Close();
		commandList->Reset(cmdAllocator, nullptr);

		return commandList;
	}

	/* ID3D12Device::CreateFence */
	inline ID3D12Fence* CreateDxFence(DevicePtr pDevice)
	{
		ID3D12Fence* fence;
		pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

		return fence;
	}

	/* ID3D12Device::CreateCommittedResource */
	/* Creates both a resource and an implicit heap, such that the heap is big enough to contain the entire resource, and the resource is mapped to the heap. */
	inline ID3D12Resource* CreateDxCommittedResource(DevicePtr device, const HelperStructs::CommittedResourceDesc& desc, D3D12_RESOURCE_STATES initialState)
	{
		D3D12_HEAP_PROPERTIES heapProperties	= desc.GetHeapProperties();
		D3D12_HEAP_FLAGS heapFlags				= desc.GetHeapFlags();
		D3D12_RESOURCE_DESC resourceDesc		= desc.GetResourceDesc();
		
		D3D12_CLEAR_VALUE optimizedClearValue	= desc.GetOptimizedClearValue();

		ID3D12Resource* committedResource;
		device->CreateCommittedResource(&heapProperties, heapFlags, &resourceDesc, initialState, &optimizedClearValue, IID_PPV_ARGS(&committedResource));

		return committedResource;
	}

	/* ID3D12CommandQueue::Signal */
	/* Signal the fence from [GPU]. At execution, the GPU will only signal this once all earlier commands are executed...*/
	inline UINT64 Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue)
	{
		uint64_t fenceValueForSignal = ++fenceValue;
		commandQueue->Signal(fence, fenceValueForSignal);

		return fenceValueForSignal;
	}

	/* ID3D12Fence::SetEventOnCompletion(fenceEvent) */
	/* Stalls the CPU thread when waiting for a fence-value to be completed. */
	inline void WaitForFenceValue(ID3D12Fence* fence, UINT64 fenceValue, HANDLE fenceEvent, float durationInMs)
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(durationInMs));
		}
	}

	/*
	* Signal()
	* WaitForFenceValue()
	*/
	/* The Flush function is used to ensure that any commands previously executed on the GPU have finished executing before the CPU thread is allowed to continue processing. */
	inline void FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue, HANDLE fenceEvent)
	{
		uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent, FLT_MAX);
	}

	/* Serialize and Create a versioned Root Signature. */
	inline ID3D12RootSignature* CreateDxSerializedRootSignature(const HelperStructs::RootSignatureDesc& rootSignatureDesc, DevicePtr pDevice) noexcept
	{
		HRESULT result{};
		ID3D12RootSignature* outSignature = nullptr;

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.Flags				= rootSignatureDesc.Flags;
		desc.NumParameters		= (UINT)rootSignatureDesc.RootParameters.size();
		desc.NumStaticSamplers	= (UINT)rootSignatureDesc.StaticSamplers.size();
		desc.pParameters		= rootSignatureDesc.RootParameters.data();
		desc.pStaticSamplers	= rootSignatureDesc.StaticSamplers.data();

		D3D12_ROOT_SIGNATURE_DESC1 desc1{};
		desc1.Flags				= rootSignatureDesc.Flags;
		desc1.NumParameters		= (UINT)rootSignatureDesc.RootParameters1.size();
		desc1.NumStaticSamplers	= (UINT)rootSignatureDesc.StaticSamplers.size();
		desc1.pParameters		= rootSignatureDesc.RootParameters1.data();
		desc1.pStaticSamplers	= rootSignatureDesc.StaticSamplers.data();

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC serializedDesc{};
		serializedDesc.Desc_1_0 = desc;
		serializedDesc.Desc_1_1 = desc1;
		serializedDesc.Version = rootSignatureDesc.MaxVersion;

		ID3DBlob* pOutBlob = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		// Serialize Root Signature:
		switch (rootSignatureDesc.MaxVersion)
		{
		case D3D_ROOT_SIGNATURE_VERSION_1_1:
			result = D3D12SerializeVersionedRootSignature(&serializedDesc, &pOutBlob, &pErrorBlob);
			break;

		case D3D_ROOT_SIGNATURE_VERSION_1_0:
			switch (serializedDesc.Version)
			{
			case D3D_ROOT_SIGNATURE_VERSION_1_0:
				// If Max version is 1_0 but the rootSignature is also 1_0, there's no problem and we can just call the D3D12 provided function:
				result = D3D12SerializeRootSignature(&serializedDesc.Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, &pOutBlob, &pErrorBlob);
				break;

			case D3D_ROOT_SIGNATURE_VERSION_1_1:
				// Else, we're gonna have to convert...
				assert(false); // Todo: But that's a nice ol' todo ;)
				break;
			}
			break;
		}

		// Create Root Signature:
		constexpr UINT nodeMask = 0u;
		result = pDevice->CreateRootSignature(nodeMask, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&outSignature));

		return outSignature;
	}

	/* Create a new GraphicsPipelineState */
	inline ID3D12PipelineState* CreateDxGraphicsPipelineState(const HelperStructs::GraphicsPipelineStateDesc& pipelineStateDesc, ID3D12RootSignature* rootSignature, DevicePtr pDevice)
	{
		HRESULT result{};
		ID3D12PipelineState* outPipelineState = nullptr;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

		psoDesc.pRootSignature = rootSignature;

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.NumElements = (UINT)pipelineStateDesc.InputElements.size();
		inputLayoutDesc.pInputElementDescs = pipelineStateDesc.InputElements.data();
		psoDesc.InputLayout = inputLayoutDesc;

		psoDesc.VS = pipelineStateDesc.VertexShaderBytecode;
		psoDesc.PS = pipelineStateDesc.PixelShaderByteCode;

		psoDesc.RasterizerState			= pipelineStateDesc.RasterizerState;
		psoDesc.BlendState				= pipelineStateDesc.BlendState;
		psoDesc.DepthStencilState		= pipelineStateDesc.DepthStencilState;
		psoDesc.SampleMask				= pipelineStateDesc.SampleMask;
		psoDesc.PrimitiveTopologyType	= pipelineStateDesc.PrimitiveTopologyType;
		psoDesc.NumRenderTargets		= (UINT)pipelineStateDesc.RenderTargetFormats.size();
		//psoDesc.RTVFormats			= pipelineStateDesc.RenderTargetFormats.data();
		psoDesc.SampleDesc				= pipelineStateDesc.SampleDesc;

		result = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outPipelineState));

		return outPipelineState;
	}

	/* Debug Layer */
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	inline void EnableDxDebugLayer()
	{
		ID3D12Debug* debugInterface;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
		debugInterface->EnableDebugLayer();
	}

	inline void DisableDxDebugLayer()
	{
		// Todo...
	}

	inline void ReportLiveObjects()
	{
		IDXGIDebug* dxgiControler;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiControler));
		dxgiControler->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
}

#endif