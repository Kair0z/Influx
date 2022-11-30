#pragma once

#ifndef _D3D12_API_H_
#define _D3D12_API_H_

#include "../GraphicsAPI.h"

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
	/* D3D12API -> RHI */
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

		virtual RHICommandQueue* CreateCommandQueue(const ERHICommandQueueType type) const override final;
		virtual RHISwapChain* CreateSwapChain(HINSTANCE windowsInstance, HWND windowHandle, RHICommandQueue* commandQueue) const override final;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHIConstantBuffer* CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const override final;

		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const override final;
		virtual RHIRenderTargetView* CreateRenderTargetView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIConstantBufferView* CreateConstantBufferView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIUnorderedAccessView* CreateUnorderedAccessView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIShaderResourceView* CreateShaderResourceView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIDepthStencilView* CreateDepthStencilView(RHIResource* resource) const override final { return nullptr; };

		virtual RHIDescriptorHeap* CreateDescriptorHeap(const ERHIDescriptorType type, uint32_t numDescriptors, bool shaderVisible = false) const override final;

		virtual RHIGraphicsPipelineLayout* CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference, RHIRenderPass* renderPass) const override final;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& fromCompiledData, ERHIShaderType shaderType, ERHIShaderModel shaderModel) const override final;

		ID3D12Device2* GetDxDevice() const;
		IDXGIAdapter4* GetDxgiAdapter() const;
		IDXGIFactory4* GetDxgiFactory() const;

		const size_t GetRTVDescriptorSize() const;
		const size_t GetDSVDescriptorSize() const;
		const size_t GetResourceDescriptorSize() const; // CBVs, UAVs, ConstantBuffers...
		const size_t GetSamplerDescriptorSize() const;
		const size_t GetDescriptorSize(const ERHIDescriptorType type) const;

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

		void CreateDescriptorOnGlobalHeap(ERHIDescriptorType type, size_t slot);

		void CreateRenderTargetViewOnGlobalHeap(size_t slot);
		void CreateConstantBufferViewOnGlobalHeap(size_t slot);
		void CreateShaderResourceViewOnGlobalHeap(size_t slot);
		void CreateUnorderedAccessViewOnGlobalHeap(size_t slot);
		void CreateSamplerOnGlobalHeap(size_t slot);
		void CreateDepthStencilViewOnGlobalHeap(size_t slot);

	public:
#pragma region D3D12StaticWrappers
		static IDXGIFactory4* CreateDxgiFactory();
		static IDXGIAdapter4* GetAdapter(IDXGIFactory4* dxgiFactory, bool useWarp);
		static ID3D12Device2* CreateDevice(IDXGIAdapter4* pAdapter);
		static ID3D12CommandQueue* CreateDxCommandQueue(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type);

		static bool CheckDxgiTearingSupport();
		static IDXGISwapChain4* CreateDxgiSwapChain(IDXGIFactory4* dxgiFactory, HWND hWnd, ID3D12CommandQueue* pCommandQueue, UINT32 w, UINT32 h, UINT32 bufferCount);
		static ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		static ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type);
		static ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device2* pDevice, ID3D12CommandAllocator* cmdAllocator, D3D12_COMMAND_LIST_TYPE type);
		static ID3D12Fence* CreateDxFence(ID3D12Device2* pDevice);

		/* An OS event handle is used to block the CPU thread until the fence has been signaled */
		static HANDLE CreateEventHandle();

		/* Signal the fence from [GPU]. At execution, the GPU will only signal this once all earlier commands are executed...*/
		static UINT64 Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue);

		/* Stalls the CPU thread when waiting for a fence-value to be completed. */
		static void WaitForFenceValue(ID3D12Fence* fence, UINT64 fenceValue, HANDLE fenceEvent, float durationInMs);

		/* The Flush function is used to ensure that any commands previously executed on the GPU have finished executing before the CPU thread is allowed to continue processing. */
		static void FlushCommandQueue(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue, HANDLE fenceEvent);

		/* Serialize A Versioned Root Signature. */
		static void SerializeVersionedRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION maxVersion, ID3DBlob** ppOutBlob, ID3DBlob** ppErrorBlob) noexcept;

		/* Debug Layer */
		/* This should be enabled only BEFORE creating the device */
		static void EnableDebugLayer();

		static void ReportLiveObjects();
#pragma endregion

	public:
		template <typename Obj>
		inline static void SafeRelease(Obj*& obj)
		{
			if (obj == nullptr) return;

			obj->Release();
			obj = nullptr;
		}
	};

	/* D3D12CommandList */
	class D3D12CommandList final : public RHICommandList
	{
	public:
		ID3D12GraphicsCommandList* GetDxCommandList()
		{
			return DxCommandList;
		}

		/* RHICommandList API: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList* cmdList)>) override final;
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue) override final;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) override final;
		virtual void BindViewports(const RHIViewport& viewport) override final;
		virtual void BindVertexBuffer(RHIVertexBuffer* vertexBuffer) override final;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) override final;
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition) override final;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) override final;
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) override final;
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) override final;
		virtual void DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation) override final;
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) override final;

	private:
		ID3D12GraphicsCommandList* DxCommandList;

		D3D12CommandList() = default;
		friend class D3D12CommandQueue;
	};

	/* D3D12CommandQueue */
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

	/* D3D12SwapChain */
	class D3D12SwapChain final : public RHISwapChain
	{
	public:
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) override final;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) override final;

		~D3D12SwapChain();

	private:
		IDXGISwapChain4* DxgiSwapChain;
		ID3D12DescriptorHeap* DxRenderTargetDescriptorHeap;

		D3D12SwapChain() = default;
		friend class D3D12API;
	};

	/* D3D12DescriptorHeap */
	class D3D12DescriptorHeap final : public RHIDescriptorHeap
	{
	public:
		D3D12DescriptorHeap() = default;
		virtual ~D3D12DescriptorHeap();

		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(size_t slot);

		ID3D12DescriptorHeap* GetDxDescriptorHeap() const;
		bool IsSlotFree(size_t slot) const;
		size_t GetFirstFreeSlot() const;

		constexpr static D3D12_CPU_DESCRIPTOR_HANDLE NullDescriptorHandle{};

	private:
		ID3D12DescriptorHeap* DxDescriptorHeap;
		std::list<size_t> OccupiedSlotIndices;
		size_t DescriptorStride;

		friend class D3D12API;
	};
}

#endif


