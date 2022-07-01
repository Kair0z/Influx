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
		virtual RHISwapChain* CreateSwapChain(HWND windowHandle, RHICommandQueue* commandQueue) const override final;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHIConstantBuffer* CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const override final;

		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const override final;
		virtual RHIRenderTargetView* CreateRenderTargetView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIConstantBufferView* CreateConstantBufferView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIUnorderedAccessView* CreateUnorderedAccessView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIShaderResourceView* CreateShaderResourceView(RHIResource* resource) const override final { return nullptr; };
		virtual RHIDepthStencilView* CreateDepthStencilView(RHIResource* resource) const override final { return nullptr; };

		virtual RHIGraphicsPipelineLayout* CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const override final;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& fromCompiledData) const override final;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const std::string& target) const override final;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const ERHIShaderType shaderType, const ERHIShaderModel shaderModel = ERHIShaderModel::SM_5_0) const override final;

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

		void CreateDescriptorOnGlobalHeap(ERHIDescriptorType type, size_t slot);

		void CreateRenderTargetViewOnGlobalHeap(size_t slot);
		void CreateConstantBufferViewOnGlobalHeap(size_t slot);
		void CreateShaderResourceViewOnGlobalHeap(size_t slot);
		void CreateUnorderedAccessViewOnGlobalHeap(size_t slot);
		void CreateSamplerOnGlobalHeap(size_t slot);
		void CreateDepthStencilViewOnGlobalHeap(size_t slot);

	public:
		/* D3D12 Static creation functions */
		/* Provides inline static functions involving creating D3D12 Objects & Resources & General functionality */
#pragma region D3D12StaticWrappers
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
		virtual void Present(bool VSync) override final;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) override final;

		~D3D12SwapChain();

	private:
		IDXGISwapChain4* DxgiSwapChain;
		ID3D12DescriptorHeap* DxRenderTargetDescriptorHeap;

		D3D12SwapChain() = default;
		friend class D3D12API;
	};

	/* D3D12DescriptorHeap */
	class D3D12DescriptorHeap final
	{
	public:
		D3D12DescriptorHeap(const ERHIDescriptorType type, size_t maxDescriptorNum, uint32_t descriptorStride);
		virtual ~D3D12DescriptorHeap();

		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(size_t slot);

		bool IsSlotFree(size_t slot) const;
		size_t GetFirstFreeSlot() const;

		constexpr static D3D12_CPU_DESCRIPTOR_HANDLE NullDescriptorHandle{};

	private:
		ID3D12DescriptorHeap* DxDescriptorHeap;
		ERHIDescriptorType Type;
		size_t MaxNumDescriptors;

		std::list<size_t> OccupiedSlotIndices;
		
		const size_t DescriptorStride;

		friend class D3D12API;
	};

	/* D3D12Shader */
	class D3D12Shader final : public RHIShader
	{
	public:
		virtual ~D3D12Shader();

	private:
		ID3DBlob* DxShaderBlob;

		D3D12Shader() = default;
		friend class D3D12API;
	};
}

#endif


