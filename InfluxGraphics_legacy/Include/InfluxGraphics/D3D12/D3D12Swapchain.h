#pragma once

#ifndef __GR_D3D12_SWAPCHAIN_H_
#define __GR_D3D12_SWAPCHAIN_H_

#include "InfluxGraphics/RHISwapchain.h"
#include "D3D12.h"

namespace influx::Graphics
{
	class D3D12Swapchain final : public RHISwapchain
	{
	private:
		friend class D3D12Device;
		D3D12Swapchain(uint32 width, uint32 height, bool isTearingSupported);

		IDXGISwapChain3* mp_dxgiSwapchain3;
		D3D12::Swapchain::ETier m_tier;

	public:
		/* Flips & Presents the backbuffer to the front-buffer. */
		// Also handles synchronization with the given RHICommandQueue
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) override final;

		/* Recreates RHISwapchain resources based on the new size */
		virtual void Resize(RHIDevice* device, RHICommandQueue* commandQueue, const math::Vectoru2& newDimensions) override final;

		IDXGISwapChain3*			GetDxgiSwapchain() const;

		virtual ~D3D12Swapchain();
	};
}

#endif
