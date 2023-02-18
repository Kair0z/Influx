#pragma once

#ifndef __GR_D3D12_DEVICE_H_
#define __GR_D3D12_DEVICE_H_

#include "InfluxGraphics/RHIDevice.h"

#include "D3D12.h"
#include "D3D12Conversion.h"

namespace Influx::Graphics
{
	class D3D12CommandQueue;
	class D3D12DescriptorHeap;
	class D3D12RenderTargetView;
	class D3D12ShaderResourceView;
	class D3D12Resource;
	class D3D12RootSignature;
	class D3D12Pipeline;

	class D3D12Device final : public RHIDevice
	{
	public:
		D3D12Device(bool enableDebug = false);
		virtual ~D3D12Device();

		/* RHIDevice API */
		virtual CommandQueuePtr CreateCommandQueue(const ERHICommandQueueType type) const override;

		virtual SwapchainPtr CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, CommandQueuePtr commandQueue) const override;

		virtual DescriptorHeapPtr CreateDescriptorHeap(const ERHIResourceViewType type, uint32 numDescriptors, bool isShaderVisible) const override;

		virtual RenderTargetViewPtr CreateRenderTargetView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const override;
		virtual ShaderResourceViewPtr CreateShaderResourceView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const override;

		virtual ResourcePtr CreateResource(const ERHIResourceState initialState) const override;
		virtual ResourcePtr CreateTextureResource(const ERHIResourceState initialState, const ERHIFormat format, const Math::Vectoru2& dimensions, const uint16 numMips) const override;

		virtual RootSignaturePtr CreateGraphicsRootSignature() const override;
		virtual PipelinePtr CreateGraphicsPipeline(const RHIPipelineDescription& desc, RootSignaturePtr rootSignature) const override;

		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) override;

		/* D3D12 API */
		ID3D12Device2* GetDxDevice() const;
		IDXGIAdapter4* GetDxgiAdapter() const;
		IDXGIFactory4* GetDxgiFactory() const;

		D3D12CommandQueue* GetGlobalGraphicsCommandQueue() const;
		D3D12CommandQueue* GetGlobalComputeCommandQueue() const;

		D3D12DescriptorHeap* GetRTVDescriptorHeap() const;
		D3D12DescriptorHeap* GetDSVDescriptorHeap() const;
		D3D12DescriptorHeap* GetResourceDescriptorHeap() const;
		D3D12DescriptorHeap* GetSamplerDescriptorHeap() const;

		/* Using GetRTVDescriptorHeap() */
		D3D12RenderTargetView* CreateRenderTargetView(const D3D12Resource* viewedResource) const;
		
		/* Using GetSRVDescriptorHeap() */
		D3D12ShaderResourceView* CreateShaderResourceView(const D3D12Resource* viewedResource) const;

		const uint64 GetRTVDescriptorSize() const;
		const uint64 GetDSVDescriptorSize() const;
		const uint64 GetResourceDescriptorSize() const; // CBVs, UAVs, ConstantBuffers...
		const uint64 GetSamplerDescriptorSize() const;
		const uint64 GetDescriptorSize(const ERHIResourceViewType type) const;

		template <typename _T>
		void Release(_T*& pointer)
		{
			if (pointer != nullptr)
			{
				pointer->Release();
				pointer = nullptr;
			}
		}

	private:
		void Initialize();
		void Cleanup();

		IDXGIFactory4* mp_dxgiFactory;
		IDXGIAdapter4* mp_dxgiAdapter;
		ID3D12Device2* mp_dxDevice;

		constexpr static bool bTearingSupported = false;
		constexpr static bool bAdditionalShadingRatesSupported = false;

		// Global Command Queues:
		void CreateGlobalQueues();
		D3D12CommandQueue* mp_graphicsQueue;
		D3D12CommandQueue* mp_computeQueue;

		// Global Descriptor Heaps:
		void CreateGlobalDescriptorHeaps();
		uint64 m_cachedRtvDescriptorSize = 0;
		uint64 m_cachedDsvDescriptorSize = 0;
		uint64 m_cachedResourceDescriptorSize = 0;
		uint64 m_cachedSamplerDescriptorSize = 0;

		// Global Descriptor Heaps
		D3D12DescriptorHeap* mp_RTVDescriptorHeap;
		D3D12DescriptorHeap* mp_resourceDescriptorHeap;
		D3D12DescriptorHeap* mp_DSVDescriptorheap;
		D3D12DescriptorHeap* mp_samplerDescriptorHeap;

		void CreateDescriptorOnGlobalHeap(ERHIResourceViewType type, uint64 slot);
		void CreateRenderTargetViewOnGlobalHeap(uint64 slot);
		void CreateConstantBufferViewOnGlobalHeap(uint64 slot);
		void CreateShaderResourceViewOnGlobalHeap(uint64 slot);
		void CreateUnorderedAccessViewOnGlobalHeap(uint64 slot);
		void CreateSamplerOnGlobalHeap(uint64 slot);
		void CreateDepthStencilViewOnGlobalHeap(uint64 slot);
	};
}

#endif