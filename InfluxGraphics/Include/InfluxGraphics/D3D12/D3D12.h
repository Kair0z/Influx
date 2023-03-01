#pragma once

#ifndef __GR_D3D12_H_
#define __GR_D3D12_H_

// Using Core...
#include "Core/BasicTypes.h"
#include "Core/String.h"
#include "Core/Container/Vector.h"

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
	using FactoryPtr = IDXGIFactory*;
	using AdapterPtr = IDXGIAdapter*;
	using DevicePtr = ID3D12Device*;
	using GraphicsCommandListPtr = ID3D12GraphicsCommandList*;
	using SwapchainPtr = IDXGISwapChain*;
	using CommandQueuePtr = ID3D12CommandQueue*;

	namespace GraphicsCommandList
	{
		enum class ETier : uint8
		{
			_0,		// ID3D12GraphicsCommandList
			_1,		// ID3D12GraphicsCommandList1
			_2,		// ID3D12GraphicsCommandList2
			_3,		// ID3D12GraphicsCommandList3
			_4,		// ID3D12GraphicsCommandList4
			_5,		// ID3D12GraphicsCommandList5
			// _6,	// ...
			_7,		// ID3D12GraphicsCommandList7
			Max
		};

		inline bool IsTierSupported(const GraphicsCommandListPtr cmdList, const ETier tier)
		{
			switch (tier)
			{
			case ETier::_0:	return (static_cast<ID3D12GraphicsCommandList*>(cmdList) != nullptr);
			case ETier::_1:	return (static_cast<ID3D12GraphicsCommandList1*>(cmdList) != nullptr);
			case ETier::_2:	return (static_cast<ID3D12GraphicsCommandList2*>(cmdList) != nullptr);
			case ETier::_3:	return (static_cast<ID3D12GraphicsCommandList3*>(cmdList) != nullptr);
			case ETier::_4:	return (static_cast<ID3D12GraphicsCommandList4*>(cmdList) != nullptr);
			case ETier::_5:	return (static_cast<ID3D12GraphicsCommandList5*>(cmdList) != nullptr);
			case ETier::_7:	return (static_cast<ID3D12GraphicsCommandList6*>(cmdList) != nullptr);
			}

			return false;
		}

		inline ETier GetMaxSupportedTier(const GraphicsCommandListPtr cmdList)
		{
			for (int t = static_cast<int>(ETier::Max); t >= 0; --t)
			{
				const ETier tier = static_cast<ETier>(t);
				if (IsTierSupported(cmdList, tier))
				{
					return tier;
				}
			}
		}
	}

	namespace Device
	{
		enum class ETier : uint8
		{
			_0,		// ID3D12Device
			_1,		// ID3D12Device1: CreatePipelineLibary() | SetEventOnMultipleFenceCompletion() | SetResidencyPriority()
			_2,		// ID3D12Device2: CreatePipelineState()
			_3,		// ID3D12Device3: EnqueueMakeResident() | OpenExistingHeapFromAddress() | OpenExistingHeapFromFileMapping()
			_4,		// ID3D12Device4: CreateCommandList1() | CreateCommittedResource1() | CreateHeap1() | CreateReservedResource1() | GetResourceAllocationInfo1() | CreateProtectedResourceSession()
			_5,		// ID3D12Device5
			_6,		// ID3D12Device6
			_7,		// ID3D12Device7
			_8,		// ID3D12Device8
			_9,		// ID3D12Device9
			Max,

			_10,		// ID3D12Device10
		};

		inline bool IsTierSupported(const DevicePtr device, const ETier tier)
		{
			switch (tier)
			{
			case ETier::_0:	return (static_cast<ID3D12Device*>(device) != nullptr);
			case ETier::_1:	return (static_cast<ID3D12Device1*>(device) != nullptr);
			case ETier::_2:	return (static_cast<ID3D12Device2*>(device) != nullptr);
			case ETier::_3:	return (static_cast<ID3D12Device3*>(device) != nullptr);
			case ETier::_4:	return (static_cast<ID3D12Device4*>(device) != nullptr);
			case ETier::_5:	return (static_cast<ID3D12Device5*>(device) != nullptr);
			case ETier::_6:	return (static_cast<ID3D12Device6*>(device) != nullptr);
			case ETier::_7:	return (static_cast<ID3D12Device7*>(device) != nullptr);
			case ETier::_8:	return (static_cast<ID3D12Device8*>(device) != nullptr);
			case ETier::_9:	return (static_cast<ID3D12Device9*>(device) != nullptr);
				// case ETier::_10:	return (static_cast<ID3D12Device10*>(device) != nullptr);
			}

			return false;
		}

		inline ETier GetMaxSupportedTier(const DevicePtr device)
		{
			for (int t = static_cast<int>(ETier::Max); t >= 0; --t)
			{
				const ETier tier = static_cast<ETier>(t);
				if (IsTierSupported(device, tier))
				{
					return tier;
				}
			}
		}

		inline DevicePtr Create(const AdapterPtr adapter, bool enableDebug)
		{
			DevicePtr device{};
			HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			
			if (enableDebug)
			{
				ID3D12InfoQueue* pInfoQueue = nullptr;
				device->QueryInterface(&pInfoQueue);
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
			}
			
			return device;
		}
	}
	
	namespace Adapter
	{
		enum class ETier : uint8
		{
			_0,
			_1,
			Max
		};

		inline bool IsTierSupported(const AdapterPtr adapter, const ETier tier)
		{
			switch (tier)
			{
			case ETier::_0:	return (static_cast<IDXGIAdapter*>(adapter) != nullptr);
			case ETier::_1:	return (static_cast<IDXGIAdapter1*>(adapter) != nullptr);
			}

			return false;
		}

		inline ETier GetMaxSupportedTier(const AdapterPtr adapter)
		{
			for (int t = static_cast<int>(ETier::Max); t >= 0; --t)
			{
				const ETier tier = static_cast<ETier>(t);
				if (IsTierSupported(adapter, tier))
				{
					return tier;
				}
			}
		}

		inline Vector<AdapterPtr> SelectAll(const FactoryPtr factory)
		{
			Vector<AdapterPtr> list{};
			AdapterPtr temp{};

			for (UINT i = 0; factory->EnumAdapters(i, &temp) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				list.push_back(temp);
			}
		}

		inline AdapterPtr Select(const FactoryPtr factory, uint8 adapterIndex = 0u)
		{
			AdapterPtr result{};

			Vector<AdapterPtr> allAdapters = SelectAll(factory);

			if (adapterIndex < allAdapters.size())
			{
				return allAdapters[adapterIndex];
			}
			else
			{
				return nullptr;
			}
		}
	}

	namespace Factory
	{
		enum class ETier : uint8
		{
			_0,
			_1,
			Max
		};

		inline bool IsTierSupported(const FactoryPtr obj, const ETier tier)
		{
			switch (tier)
			{
			case ETier::_0:	return (static_cast<IDXGIFactory*>(obj) != nullptr);
			case ETier::_1:	return (static_cast<IDXGIFactory1*>(obj) != nullptr);
			}

			return false;
		}

		inline ETier GetMaxSupportedTier(const FactoryPtr obj)
		{
			for (int t = static_cast<int>(ETier::Max); t >= 0; --t)
			{
				const ETier tier = static_cast<ETier>(t);
				if (IsTierSupported(obj, tier))
				{
					return tier;
				}
			}
		}

		inline FactoryPtr Create()
		{
			FactoryPtr result = nullptr;
			::CreateDXGIFactory(IID_PPV_ARGS(&result));
			return result;
		}

		inline IDXGIFactory2* CreateTier2(bool debug)
		{
			IDXGIFactory2* result = nullptr;
			UINT flags = (debug) ? DXGI_CREATE_FACTORY_DEBUG : 0u;

			::CreateDXGIFactory2(flags, IID_PPV_ARGS(&result));
			return result;
		}
	}

	namespace Swapchain
	{
		enum class ETier : uint8
		{
			_0,
			_1,
			Max
		};

		inline bool IsTierSupported(const SwapchainPtr obj, const ETier tier)
		{
			switch (tier)
			{
			case ETier::_0:	return (static_cast<IDXGISwapChain*>(obj) != nullptr);
			case ETier::_1:	return (static_cast<IDXGISwapChain1*>(obj) != nullptr);
			}

			return false;
		}

		inline ETier GetMaxSupportedTier(const SwapchainPtr obj)
		{
			for (int t = static_cast<int>(ETier::Max); t >= 0; --t)
			{
				const ETier tier = static_cast<ETier>(t);
				if (IsTierSupported(obj, tier))
				{
					return tier;
				}
			}
		}

		/*
		* REQUIRES
		* IDXGISwapChain1
		* IDXGIFactory2
		* 
		* USES:
		* IDXGIFactory2::CreateSwapChainForHwnd
		* IDXGIFactory2::MakeWindowAssociation
		* DXGI_SWAP_CHAIN_DESC1
		*/
		inline IDXGISwapChain1* CreateTier1(IDXGIFactory2* dxgiFactory, ::HWND hWnd, CommandQueuePtr pCommandQueue,
			uint32 w, uint32 h, uint8 numBuffers, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			IDXGISwapChain1* dxgiSwapChain1;
			UINT flags = 0;

			DXGI_SWAP_CHAIN_DESC1 desc{};
			desc.Width = w;
			desc.Height = h;
			desc.Format = format;
			desc.Stereo = false;
			desc.SampleDesc = { 1, 0 };
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = numBuffers;
			desc.Scaling = DXGI_SCALING_STRETCH;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			desc.Flags = Query::SupportsTearing() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

			dxgiFactory->CreateSwapChainForHwnd(pCommandQueue, hWnd, &desc, nullptr, nullptr, &dxgiSwapChain1);

			// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
			// will be handled manually.
			dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
			return dxgiSwapChain1;
		}

		inline IDXGISwapChain2* CreateTier2(IDXGIFactory2* dxgiFactory, ::HWND hWnd, CommandQueuePtr pCommandQueue,
			uint32 w, uint32 h, uint8 numBuffers, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			return static_cast<IDXGISwapChain2*>(CreateTier1(dxgiFactory, hWnd, pCommandQueue, w, h, numBuffers, format));
		}

		inline IDXGISwapChain3* CreateTier3(IDXGIFactory2* dxgiFactory, ::HWND hWnd, CommandQueuePtr pCommandQueue,
			uint32 w, uint32 h, uint8 numBuffers, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			return static_cast<IDXGISwapChain3*>(CreateTier1(dxgiFactory, hWnd, pCommandQueue, w, h, numBuffers, format));
		}
	}

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
			bool m_allowOptimizedClearValue = false;

		public:
			static CommittedResourceDesc AsTexture(const DXGI_FORMAT format, uint64_t width, uint64_t height, uint16_t numMipLevels)
			{
				CommittedResourceDesc desc{};

				desc.m_heapProperties		= GetDefaultHeapProperties();
				desc.m_heapFlags			= D3D12_HEAP_FLAG_NONE;
				desc.m_resourceDesc			= GetTextureDesc(format, width, height, numMipLevels);
				desc.m_optimizedClearValue	= GetOptimizedClearValue(format);
				desc.m_allowOptimizedClearValue = true;

				return desc;
			}

			static CommittedResourceDesc AsBuffer(bool useUploadHeap, uint64 totalBufferSizeInBytes, uint64_t alignment)
			{
				CommittedResourceDesc desc{};

				desc.m_heapProperties			= useUploadHeap ? GetUploadHeapProperties() : GetDefaultHeapProperties();
				desc.m_heapFlags				= D3D12_HEAP_FLAG_NONE;
				desc.m_resourceDesc				= GetBufferDesc(totalBufferSizeInBytes, alignment);
				desc.m_allowOptimizedClearValue	= false;

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

			static D3D12_HEAP_PROPERTIES GetUploadHeapProperties()
			{
				D3D12_HEAP_PROPERTIES defaultProperties;
				defaultProperties.Type					= D3D12_HEAP_TYPE_UPLOAD;
				defaultProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				defaultProperties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
				defaultProperties.CreationNodeMask		= 1;
				defaultProperties.VisibleNodeMask		= 1;
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
				textureDesc.Width				= width;
				textureDesc.Height				= (UINT)height;
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

			static D3D12_RESOURCE_DESC GetBufferDesc(uint64_t width, uint64_t alignment = 0u)
			{
				D3D12_RESOURCE_DESC bufferDesc{};

				bufferDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
				bufferDesc.Format				= DXGI_FORMAT_UNKNOWN;
				bufferDesc.Width				= width;
				bufferDesc.Height				= 1u;
				bufferDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
				bufferDesc.DepthOrArraySize		= 1u;
				bufferDesc.MipLevels			= 1u;
				bufferDesc.SampleDesc.Count		= 1u;
				bufferDesc.SampleDesc.Quality	= 0u;
				bufferDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				bufferDesc.Alignment			= alignment;

				return bufferDesc;
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

			bool AllowOptimizedClearValue() const
			{
				return m_allowOptimizedClearValue;
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
			D3D12_SHADER_BYTECODE DomainShaderByteCode;
			D3D12_SHADER_BYTECODE HullShaderByteCode;
			D3D12_SHADER_BYTECODE GeometryShaderByteCode;

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

	namespace Query
	{
		// GraphicsCommandList >= ETier::_4
		inline bool SupportsRenderPasses(const GraphicsCommandListPtr commandList)
		{
			using namespace GraphicsCommandList;
			return IsTierSupported(commandList, ETier::_4);
		}

		inline bool SupportsTearing()
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
		device->CreateCommittedResource(&heapProperties, heapFlags, &resourceDesc, initialState, 
			desc.AllowOptimizedClearValue() ? &optimizedClearValue : nullptr, IID_PPV_ARGS(&committedResource));

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
		psoDesc.DS = pipelineStateDesc.DomainShaderByteCode;
		psoDesc.HS = pipelineStateDesc.HullShaderByteCode;
		psoDesc.GS = pipelineStateDesc.GeometryShaderByteCode;

		psoDesc.RasterizerState			= pipelineStateDesc.RasterizerState;
		psoDesc.BlendState				= pipelineStateDesc.BlendState;
		psoDesc.DepthStencilState		= pipelineStateDesc.DepthStencilState;
		psoDesc.SampleMask				= pipelineStateDesc.SampleMask;
		psoDesc.PrimitiveTopologyType	= pipelineStateDesc.PrimitiveTopologyType;
		psoDesc.NumRenderTargets		= (UINT)pipelineStateDesc.RenderTargetFormats.size();
		psoDesc.SampleDesc				= pipelineStateDesc.SampleDesc;

		for (uint8 rt = 0u; rt < pipelineStateDesc.RenderTargetFormats.size(); ++rt)
		{
			psoDesc.RTVFormats[rt] = pipelineStateDesc.RenderTargetFormats[rt];
		}

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