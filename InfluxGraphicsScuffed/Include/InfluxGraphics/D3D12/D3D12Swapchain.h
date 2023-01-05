#pragma once

#ifndef __GR_D3D12_SWAPCHAIN_H_
#define __GR_D3D12_SWAPCHAIN_H_

#include "InfluxGraphics/RHISwapchain.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	class D3D12Swapchain final : public RHISwapchain
	{
	private:
		friend class D3D12Device;
		D3D12Swapchain(uint32 width, uint32 height, bool isTearingSupported);

		IDXGISwapChain4* mp_dxgiSwapchain;
		ID3D12DescriptorHeap* mp_dxRenderTargetDescriptorHeap;

	public:
		/* Flips & Presents the backbuffer to the front-buffer. */
		// Also handles synchronization with the given RHICommandQueue
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) override final;

		/* Recreates RHISwapchain resources based on the new size */
		virtual void Resize(RHIDevice* device, RHICommandQueue* commandQueue, const Math::Vectoru2& newDimensions) override final;

		ID3D12DescriptorHeap* GetDxRtvDescriptorHeap() const;

		virtual ~D3D12Swapchain();
	};
}

#endif
