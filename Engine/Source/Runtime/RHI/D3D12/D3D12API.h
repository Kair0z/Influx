#pragma once
#include "Runtime/RHI/RenderAPI.h"

#ifndef _D3D12_API_H_
#define _D3D12_API_H_

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "D3DX12/d3dx12.h"

#include <chrono>

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

#include "Core/Geometry/Vertex.h"
#include "Runtime/RHI/RHITypes.h"
#include "Core/Assert/Assert.h"

namespace Influx
{
	inline DXGI_FORMAT ToDxgi(ERHIFormat format)
	{
		switch (format) {
		case ERHIFormat::D_32_Float: return DXGI_FORMAT_D32_FLOAT;
		case ERHIFormat::RGBA_32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ERHIFormat::RGBA_8_Unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		default: return DXGI_FORMAT_UNKNOWN;
		}
	}

	inline D3D12_RESOURCE_FLAGS ToD3D12(const ERHIResourceFlags flags)
	{
		switch (flags)
		{
		case ERHIResourceFlags::None: return D3D12_RESOURCE_FLAG_NONE;
		case ERHIResourceFlags::AllowRenderTarget: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		case ERHIResourceFlags::AllowDepthStencil: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		case ERHIResourceFlags::AllowUnorderedAccess: return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		case ERHIResourceFlags::DenyShaderResource: return D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		case ERHIResourceFlags::AllowCrossAdapter: return D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
		case ERHIResourceFlags::AllowSimultaneousAccess: return D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		case ERHIResourceFlags::VideoDecodeReferenceOnly: return D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY;
		case ERHIResourceFlags::VideoEncodeReferenceOnly: return D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY;
		default: return D3D12_RESOURCE_FLAG_NONE;
		}
	}

	inline D3D12_RESOURCE_STATES ToD3D12(const ERHIResourceState state)
	{
		switch (state)
		{
		case ERHIResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
		case ERHIResourceState::VertexAndConstantBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case ERHIResourceState::IndexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case ERHIResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case ERHIResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ERHIResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case ERHIResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
		case ERHIResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
		case ERHIResourceState::RaytracingAS: return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		case ERHIResourceState::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
		case ERHIResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ERHIResourceState::GenericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
		case ERHIResourceState::AllShaderResource: return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		case ERHIResourceState::NonPixelReadResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case ERHIResourceState::PixelShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		default: return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	class D3D12RootSignature;

	/* API that encapsulates D3D12 Device & Maintains D3D12 globally */
	class D3D12API final : public RenderAPI
	{
	public:
		//virtual Ptr<Buffer> CreateBuffer(const Buffer::Initializer& init) override;
		//virtual Ptr<Buffer> CreateVertexBuffer(const Buffer::Initializer& init) override;
		//virtual Ptr<Buffer> CreateIndexBuffer(const Buffer::Initializer& init) override;
		//virtual Ptr<Shader> CreateVertexShader(const String& filepath) override;
		//virtual Ptr<Shader> CreatePixelShader(const String& filepath) override;
		virtual Ptr<RHIRenderTarget> CreateRenderTarget(const Vector2u& dimensions, const ERHIFormat format) override;
		virtual Ptr<RHIRenderTarget> CreateDepthStencilTarget(const Vector2u& dimensions, const ERHIFormat format) override;
		virtual Ptr<RHIGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineBuilder& desc) override;
		virtual Ptr<RHISwapChain> CreateSwapChain(const SwapChainDesc& desc, const Ptr<RHICommandQueue> commandQueue) override;
		virtual Ptr<RHICommandQueue> CreateCommandQueue(const CommandQueueDesc& desc) override;
		virtual void SetupDebugLayer() override;

		D3D12API() = default;
		~D3D12API();

		Ptr<D3D12RootSignature> CreateRootSignature(const struct D3D12RootSignatureDesc& desc);

		virtual void Initialize() override final;

		static constexpr bool GetShouldUseWarp() { return sShouldUseWarp; };
		ID3D12Device2* GetDevice() const;

	private:
		/* Windows Advanced Rasterization Platform (WARP) ...*/
		constexpr static bool sShouldUseWarp = true;
		ID3D12Device2* mpDevice{};
		IDXGIAdapter4* mpAdapter{};

	public:
#pragma region Statics
		/*
		*	[D3D12Statics]
		*	Provides inline static functions involving creating D3D12 Objects & Resources & General functionality
		*/
		/* Query a compatible adapter */
		inline static IDXGIAdapter4* GetAdapter(bool useWarp)
		{
			/* Create Factory... */
			IDXGIFactory4* dxgiFactory;
			UINT flags = 0;
#ifdef _DEBUG
			flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
			/* TODO: Throw On Fail... */
			CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory));

			/* Get Adapter ...*/
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

		/* Create D3D12Device */
		inline static ID3D12Device2* CreateDevice(IDXGIAdapter4* pAdapter)
		{
			ID3D12Device2* d3d12Device2{};
			D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));

#ifdef _DEBUG
			ID3D12InfoQueue* pInfoQueue;
			if (pInfoQueue = (ID3D12InfoQueue*)d3d12Device2)
			{
				/* TODO: Why does this crash? */
				/*pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);*/

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

				//pInfoQueue->PushStorageFilter(&NewFilter);
			}
#endif
			return d3d12Device2;
		}

		/* Create D3D12CommandQueue */
		inline static ID3D12CommandQueue* CreateCommandQueue(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type)
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

		/* Create Swap-Chain */
		inline static bool CheckTearingSupport()
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
		inline static IDXGISwapChain4* CreateSwapChain(HWND hWnd, ID3D12CommandQueue* pCommandQueue, uint32_t w, uint32_t h, uint32_t bufferCount)
		{
			IDXGISwapChain4* dxgiSwapChain4;
			IDXGIFactory4* dxgiFactory4;
			UINT flags = 0;
#ifdef _DEBUG
			flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
			CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory4));

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
			desc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
			
			IDXGISwapChain1* swapChain1;
			dxgiFactory4->CreateSwapChainForHwnd(pCommandQueue, hWnd, &desc, nullptr, nullptr, &swapChain1);

			// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
			// will be handled manually.
			dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

			dxgiSwapChain4 = (IDXGISwapChain4*)swapChain1;
			return dxgiSwapChain4;
		}

		/* Create Descriptor Heap */
		inline static ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
		{
			ID3D12DescriptorHeap* descHeap;

			D3D12_DESCRIPTOR_HEAP_DESC desc{};
			desc.NumDescriptors = numDescriptors;
			desc.Type = type;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap));
			return descHeap;
		}

		/* Create Command Allocator */
		inline static ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type)
		{
			ID3D12CommandAllocator* cmdAllocator;
			pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&cmdAllocator));
			return cmdAllocator;
		}

		/* Create Graphics Command List */
		inline static ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device2* pDevice, ID3D12CommandAllocator* cmdAllocator, D3D12_COMMAND_LIST_TYPE type)
		{
			ID3D12GraphicsCommandList* commandList;
			pDevice->CreateCommandList(0, type, cmdAllocator, nullptr, IID_PPV_ARGS(&commandList));
			commandList->Close();
			commandList->Reset(cmdAllocator, nullptr);
			return commandList;
		}

		/* Create Fence */
		inline static ID3D12Fence* CreateFence(ID3D12Device2* pDevice)
		{
			ID3D12Fence* fence;
			pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

			return fence;
		}

		/* An OS event handle is used to block the CPU thread until the fence has been signaled */
		inline static HANDLE CreateEventHandle()
		{
			HANDLE fenceEvent;

			fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			ASSERT(fenceEvent && "Failed to create fence event.");

			return fenceEvent;
		}

		/* Signal the fence from [GPU]. At execution, the GPU will only signal this once all earlier commands are executed...*/
		inline static uint64_t Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence,
			uint64_t& fenceValue)
		{
			uint64_t fenceValueForSignal = ++fenceValue;
			commandQueue->Signal(fence, fenceValueForSignal);

			return fenceValueForSignal;
		}

		/* Stalls the CPU thread when waiting for a fence-value to be completed. */
		inline static void WaitForFenceValue(ID3D12Fence* fence, uint64_t fenceValue, HANDLE fenceEvent,
			std::chrono::milliseconds duration = std::chrono::milliseconds::max())
		{
			if (fence->GetCompletedValue() < fenceValue)
			{
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
			}
		}

		/* The Flush function is used to ensure that any commands previously executed on the GPU have finished executing before the CPU thread is allowed to continue processing. */
		inline static void Flush(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence,
			uint64_t& fenceValue, HANDLE fenceEvent)
		{
			uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
			WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
		}

		/* Debug Layer */
		/* This should be enabled only BEFORE creating the device */
		inline static void EnableDebugLayer()
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

		/* Transition Resource */
		inline static void TransitionResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
			D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				resource, before, after);

			commandList->ResourceBarrier(1, &barrier);
		}

		/* Clear RTV */
		inline static void ClearRenderTargetView(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
		{
			commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}

		/* Resource View Sizes */
		inline static size_t GetDescriptorHandleIncrementSize_CBV_SRV_UAV(ID3D12Device* pDevice)
		{
			static const size_t size = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			return size;
		}
		inline static size_t GetDescriptorHandleIncrementSize_DSV(ID3D12Device* pDevice)
		{
			static const size_t size = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			return size;
		}
		inline static size_t GetDescriptorHandleIncrementSize_RTV(ID3D12Device* pDevice)
		{
			static const size_t size = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			return size;
		}
#pragma endregion
	};
}

#endif


