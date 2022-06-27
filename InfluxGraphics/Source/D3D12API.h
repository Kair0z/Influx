#pragma once

#ifndef _D3D12_API_H_
#define _D3D12_API_H_

#include "GraphicsAPI.h"

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

#include <queue>
#include <list>

namespace Influx::Graphics
{
	namespace Conversion
	{
		constexpr D3D12_COMMAND_LIST_TYPE ToDx12(ECommandQueueType type)
		{
			switch (type)
			{
			default:
			case ECommandQueueType::Graphics:
				return D3D12_COMMAND_LIST_TYPE_DIRECT;
			}
		}

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE ToDx12(ERHIDescriptorType type)
		{
			switch (type)
			{
			case ERHIDescriptorType::DSV: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			case ERHIDescriptorType::Resource: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			case ERHIDescriptorType::RTV: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			case ERHIDescriptorType::Sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

			default:
			case ERHIDescriptorType::Invalid: return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
			}
		}

		constexpr D3D12_RESOURCE_STATES ToDx12(const ERHIResourceState state)
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

		constexpr DXGI_FORMAT ToDx12(ERHIFormat format)
		{
			switch (format) {
			case ERHIFormat::D_32_Float: return DXGI_FORMAT_D32_FLOAT;
			case ERHIFormat::RGBA_32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ERHIFormat::RGBA_8_Unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		constexpr D3D12_PRIMITIVE_TOPOLOGY ToDx12(const ERHIPrimitiveTopology topology)
		{
			switch (topology)
			{
			default:
			case ERHIPrimitiveTopology::TriangleList: return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			}
		}
	}

	class D3D12API final : public GraphicsAPI
	{
		/* Private constructor -> Singleton */
		D3D12API();

	public:
		/* Singleton Object holding references to ID3D12Device, IDXGIFactory, ... */
		static D3D12API& Get()
		{
			static D3D12API api{};
			return api;
		}
		virtual ~D3D12API();

		virtual RHICommandQueue* CreateCommandQueue(const ECommandQueueType type) const override final;
		virtual RHISwapChain* CreateSwapChain(HWND windowHandle, RHICommandQueue* commandQueue) const override final;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const override final;
		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const override final;

		ID3D12Device2* GetDxDevice() const;
		IDXGIAdapter4* GetDxgiAdapter() const;
		IDXGIFactory4* GetDxgiFactory() const;

		const size_t GetRTVDescriptorSize() const;
		const size_t GetDSVDescriptorSize() const;
		const size_t GetResourceDescriptorSize() const; // CBVs, UAVs, ConstantBuffers...
		const size_t GetSamplerDescriptorSize() const;

	private:
		IDXGIFactory4* DxgiFactory;
		IDXGIAdapter4* DxgiAdapter;
		ID3D12Device2* DxDevice;

		constexpr static bool bTearingSupported = false;
		constexpr static bool bAdditionalShadingRatesSupported = false;

		void CreateGlobalDescriptorHeaps();
		size_t CachedRtvDescriptorSize = 0;
		size_t CachedDsvDescriptorSize = 0;
		size_t CachedResourceDescriptorSize = 0;
		size_t CachedSamplerDescriptorSize = 0;

		// Global Descriptor Heaps
		class D3D12DescriptorHeap* RTVDescriptorHeap;
		class D3D12DescriptorHeap* ResourceDescriptorHeap;
		class D3D12DescriptorHeap* DSVDescriptorheap;
		class D3D12DescriptorHeap* SamplerDescriptorHeap;

	public:
		/* D3D12 Static creation functions */
		/* Provides inline static functions involving creating D3D12 Objects & Resources & General functionality */
		static IDXGIFactory4* CreateDxgiFactory();

		/* Query a compatible adapter */
		static IDXGIAdapter4* GetAdapter(IDXGIFactory4* dxgiFactory, bool useWarp);

		/* Create D3D12Device */
		static ID3D12Device2* CreateDevice(IDXGIAdapter4* pAdapter);

		/* Create D3D12CommandQueue */
		static ID3D12CommandQueue* CreateDxCommandQueue(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type);

		/* Create Swap-Chain */
		static bool CheckDxgiTearingSupport();
		static IDXGISwapChain4* CreateDxgiSwapChain(IDXGIFactory4* dxgiFactory, HWND hWnd, ID3D12CommandQueue* pCommandQueue, UINT32 w, UINT32 h, UINT32 bufferCount);

		/* Create Descriptor Heap */
		static ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors);

		/* Create Command Allocator */
		static ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type);

		/* Create Graphics Command List */
		static ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device2* pDevice, ID3D12CommandAllocator* cmdAllocator, D3D12_COMMAND_LIST_TYPE type);

		/* Create Fence */
		static ID3D12Fence* CreateDxFence(ID3D12Device2* pDevice);

		/* An OS event handle is used to block the CPU thread until the fence has been signaled */
		static HANDLE CreateEventHandle();

		/* Signal the fence from [GPU]. At execution, the GPU will only signal this once all earlier commands are executed...*/
		static UINT64 Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue);

		/* Stalls the CPU thread when waiting for a fence-value to be completed. */
		static void WaitForFenceValue(ID3D12Fence* fence, UINT64 fenceValue, HANDLE fenceEvent, float durationInMs);

		/* The Flush function is used to ensure that any commands previously executed on the GPU have finished executing before the CPU thread is allowed to continue processing. */
		static void FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue, HANDLE fenceEvent);

		/* Debug Layer */
		/* This should be enabled only BEFORE creating the device */
		static void EnableDebugLayer();

		static void ReportLiveObjects();

	public:
		template <typename Obj>
		inline static void SafeRelease(Obj* obj)
		{
			if (obj != nullptr)
			{
				obj->Release();
				obj = nullptr;
			}
		}
	};

	class D3D12CommandList final : public RHICommandList
	{
	public:
		ID3D12GraphicsCommandList* GetDxCommandList()
		{
			return DxCommandList;
		}

		/* RHICommandList API: */
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue) override final;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) override final;
		virtual void BindViewports(const RHIViewport& viewport) override final;
		virtual void BindVertexBuffer(RHIVertexBuffer* vertexBuffer) override final;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) override final;
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition) override final;

	private:
		ID3D12GraphicsCommandList* DxCommandList;

		D3D12CommandList() = default;
		friend class D3D12CommandQueue;
	};

	class D3D12CommandQueue final : public RHICommandQueue
	{
	public:
		virtual RHICommandList* SetupNewCommandList(GraphicsAPI* api) override final;
		virtual void ExecuteCommmandList(RHICommandList* commandList) override final;
		virtual void Flush() override final;

		~D3D12CommandQueue();

	private:
		struct CommandAllocatorEntry
		{
			UINT64 FenceValue;
			ID3D12CommandAllocator* Allocator;
		};

		ID3D12CommandQueue* DxCommandQueue;
		ID3D12Fence* DxFence;
		std::queue<CommandAllocatorEntry> CommandAllocatorQueue;
		std::queue<D3D12CommandList*> CommandListQueue;
		HANDLE FenceEventHandle;
		UINT64 CurrentFenceValue;

		bool IsFenceComplete(UINT64 completeValue) const;

		D3D12CommandQueue() = default;
		friend class D3D12API;
	};

	class D3D12SwapChain final : public RHISwapChain
	{
	public:
		virtual void Present(bool VSync) override final;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) override final;

	private:
		IDXGISwapChain4* DxgiSwapChain;
		ID3D12Resource* DxBackBufferResources[NumBackBuffers];
		ID3D12DescriptorHeap* DxRenderTargetDescriptorHeap;

		D3D12SwapChain() = default;
		friend class D3D12API;
	};

	class D3D12Texture final : public RHITexture
	{
	public:
		~D3D12Texture() = default;

	private:
		ID3D12DescriptorHeap* DxDescriptorHeap;
		int DescriptorHeapIndex;

		D3D12Texture() = default;
		friend class D3D12API;
	};

	class D3D12Resource final : public RHIResource
	{
	public:
		D3D12Resource();
		D3D12Resource(ID3D12Resource* dxResource, ERHIResourceState initialState);
		ID3D12Resource* GetDxResource() const;

	private:
		ID3D12Resource* DxResource;
		friend class D3D12API;
	};

	class D3D12DescriptorHeap final
	{
	public:
		D3D12DescriptorHeap(const ERHIDescriptorType type, UINT64 maxDescriptorNum);

	private:
		ID3D12DescriptorHeap* DxDescriptorHeap;
		ERHIDescriptorType Type;
		UINT64 MaxNumDescriptors;

		std::list<UINT64> FreeIndices;
		
		D3D12DescriptorHeap() = default;
		friend class D3D12API;
	};

	class D3D12RenderTargetView final : public RHIRenderTargetView
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE DxCPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE DxGPUHandle;
	};

	class D3D12VertexBuffer final : public RHIVertexBuffer
	{
	public:
		D3D12VertexBuffer(D3D12Resource* gpuResource);
		D3D12_VERTEX_BUFFER_VIEW GetDxVertexBufferView() const;

	private:
		D3D12_VERTEX_BUFFER_VIEW DxVertexBufferView;

		D3D12VertexBuffer() = default;
		friend class D3D12API;
	};
}

#endif


