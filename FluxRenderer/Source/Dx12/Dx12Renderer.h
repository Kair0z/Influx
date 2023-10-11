#pragma once

#include "../Renderer/IFluxRenderer.h"

struct IDXGISwapChain3;
struct ID3D12Device;
struct IDXGIAdapter;
struct ID3D12Resource;
struct IDXGIFactory2;
struct ID3D12CommandQueue;
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12Fence;

namespace influx
{
	class Dx12Renderer final : public IFluxRenderer
	{
	public:
		Dx12Renderer() = default;

	private:
		virtual void RecordRenderCommands(platform::window_handle windowHandle) override final;

		virtual void PresentToWindow(platform::window_handle windowHandle) override final;

		void WaitForPreviousFrame();

	private:
		void Initialize();
		void InitializeDevice();
		void InitializeCommandQueue();
		void InitializeDescriptorHeaps();
		void InitializeCommandList();
		void InitializePipeline();
		void InitializeSceneDataBuffers();
		void InitializeShaderResourceDataBuffers();
		void InitializeSamplers();
		void InitializeLights();
		void InitializeSynchronization();

		void InitializeSwapchain(platform::window_handle windowHandle);

		IDXGIFactory2* mp_dxgiFactory2;
		IDXGIAdapter* mp_dxgiHardwareAdapter;
		ID3D12Device* mp_device;

		ID3D12CommandQueue* mp_commandQueue;
		IDXGISwapChain3* mp_swapchain;
		ID3D12Resource* mp_swapchainBufferResources[GetNumSwapchainBuffers()];
		uint64 m_currentSwapchainBufferIndex;

		ID3D12CommandAllocator* mp_commandAllocators[GetNumSwapchainBuffers()];
		ID3D12GraphicsCommandList* mp_gfxCommandLists[GetNumSwapchainBuffers()];

		ID3D12DescriptorHeap* mp_rtvDescriptorHeap;
		ID3D12DescriptorHeap* mp_dsvDescriptorHeap;
		ID3D12DescriptorHeap* mp_resDescriptorHeap; // UAV, CBV, SRV
		ID3D12DescriptorHeap* mp_samplerDescriptorHeap;

		uint64 m_rtvDescriptorSize;
		uint64 m_dsvDescriptorSize;
		uint64 m_resDescriptorSize;
		uint64 m_samplerDescriptorSize;

		ID3D12RootSignature* mp_rootSignature;
		ID3D12PipelineState* mp_pipelineState;

		ID3D12Resource* mp_vertexBufferResource;
		ID3D12Resource* mp_indexBufferResource;

		// Synchronization
		uint32 m_frameIndex;
		void* m_fenceEvent;
		ID3D12Fence* mp_fence;
		uint64 m_fenceValue;
	};
}


