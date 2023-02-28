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
	class D3D12GraphicsPipelineLayout;
	class D3D12GraphicsPipeline;

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
		virtual ResourcePtr CreateVertexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const override;
		virtual ResourcePtr CreateIndexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const override;

		virtual GraphicsPipelineLayoutPtr CreateGraphicsPipelineLayout() const override;
		virtual GraphicsPipelinePtr CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& desc, GraphicsPipelineLayoutPtr rootSignature) const override;

		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) override;

		/* D3D12 API */
		ID3D12Device2* GetDxDevice() const;
		IDXGIAdapter4* GetDxgiAdapter() const;
		IDXGIFactory4* GetDxgiFactory() const;

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
	};
}

#endif