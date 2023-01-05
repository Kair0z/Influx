#pragma once

#ifndef __GR_D3D12_DEVICE_H_
#define __GR_D3D12_DEVICE_H_

#include "InfluxGraphics/RHIDevice.h"

#include "D3D12.h"
#include "D3D12Conversion.h"

namespace Influx::Graphics
{
	class D3D12DescriptorHeap;
	
	class D3D12Device final : public RHIDevice
	{
	public:
		D3D12Device(bool enableDebug = false);
		virtual ~D3D12Device();

		/* Creating API objects & Resources */
		virtual RHICommandQueue* CreateCommandQueue(const ERHICommandQueueType type) const override;

		virtual RHISwapchain* CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, RHICommandQueue* commandQueue) const override;

		virtual RHIDescriptorHeap* CreateDescriptorHeap(const ERHIDescriptorType type, uint32 numDescriptors, bool isShaderVisible) const override;

		virtual RHIResource* CreateResource() const override;

		/* Debug Layer*/
		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) override;

		ID3D12Device2* GetDxDevice() const;
		IDXGIAdapter4* GetDxgiAdapter() const;
		IDXGIFactory4* GetDxgiFactory() const;

		const uint64 GetRTVDescriptorSize() const;
		const uint64 GetDSVDescriptorSize() const;
		const uint64 GetResourceDescriptorSize() const; // CBVs, UAVs, ConstantBuffers...
		const uint64 GetSamplerDescriptorSize() const;
		const uint64 GetDescriptorSize(const ERHIDescriptorType type) const;

	private:
		void Initialize();
		void Cleanup();

		IDXGIFactory4* mp_dxgiFactory;
		IDXGIAdapter4* mp_dxgiAdapter;
		ID3D12Device2* mp_dxDevice;

		constexpr static bool bTearingSupported = false;
		constexpr static bool bAdditionalShadingRatesSupported = false;

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

		void CreateDescriptorOnGlobalHeap(ERHIDescriptorType type, uint64 slot);
		void CreateRenderTargetViewOnGlobalHeap(uint64 slot);
		void CreateConstantBufferViewOnGlobalHeap(uint64 slot);
		void CreateShaderResourceViewOnGlobalHeap(uint64 slot);
		void CreateUnorderedAccessViewOnGlobalHeap(uint64 slot);
		void CreateSamplerOnGlobalHeap(uint64 slot);
		void CreateDepthStencilViewOnGlobalHeap(uint64 slot);
	};
}

#endif