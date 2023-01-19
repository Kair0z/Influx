#pragma once

#ifndef __GR_RHI_SWAPCHAIN_H_
#define __GR_RHI_SWAPCHAIN_H_

#include "Types.h"
#include "RHITypes.h"
#include "Core/Math/Vector.h"

#include "Core/Platform/Platform.h"

namespace Influx::Graphics
{
	class RHICommandQueue;
	class RHIDevice;
	class RHIResource;
	class RHIRenderTargetView;

	/* Swapchain */
	class RHISwapchain
	{
		constexpr static uint8 k_numBackBuffers = 3;

	protected:
		RHISwapchain(uint32 width, uint32 height, bool isTearingSupported);

		RHIResource* mp_backBufferResources[k_numBackBuffers];
		RHIRenderTargetView* mp_backBufferRTVs[k_numBackBuffers];

		ERHIFormat m_renderTargetFormat = ERHIFormat::INVALID;

		uint32 m_currentBackBufferIndex = 0;
		uint32 m_width = 0;
		uint32 m_height = 0;

		bool m_isTearingSupported = false;

		Platform::WindowHandle m_windowHandle;

	public:
		/* Flips & Presents the backbuffer to the front-buffer. */
		// Also handles synchronization with the given RHICommandQueue
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) = 0;

		/* Recreates RHISwapchain resources based on the new size */
		virtual void Resize(RHIDevice* device, RHICommandQueue* commandQueue, const Math::Vectoru2& newDimensions) = 0;

		RHIResource* GetCurrentBackBufferResource();
		RHIRenderTargetView* GetCurrentRenderTargetView();

		const uint32 GetCurrentBackBufferIndex() const { return m_currentBackBufferIndex; }
		const uint32 GetWidth() const { return m_width; }
		const uint32 GetHeight() const { return m_height; }

		Platform::WindowHandle GetWindowHandle() const;

		bool GetIsTearingSupported() const;

		ERHIFormat GetRenderTargetFormat() const;

		constexpr static uint8 GetNumBackBuffers() { return k_numBackBuffers; }

	public:
		RHISwapchain(const RHISwapchain&) = delete;
		RHISwapchain(RHISwapchain&&) = delete;
		RHISwapchain& operator=(const RHISwapchain&) = delete;
		RHISwapchain& operator=(RHISwapchain&&) = delete;
		virtual ~RHISwapchain() = default;
	};
}

#endif