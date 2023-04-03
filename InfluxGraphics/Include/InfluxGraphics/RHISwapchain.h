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

	class IRHISwapchain final
	{
	public:
		/* Number of buffers to use in the swapchain */
		enum class EFrameBuffering : uint8
		{
			Single = 1,
			Double = 2,
			Triple = 3,
			Max
		};

		/* Flips & Presents the backbuffer to the front-buffer. */
		// Also handles synchronization with the given RHICommandQueue
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) = 0;

		/* Recreates RHISwapchain resources based on the new size */
		virtual void Resize(RHIDevice* device, RHICommandQueue* commandQueue, const Math::Vectoru2& newDimensions) = 0;

		RHIResource* GetCurrentBackBufferResource();
		RHIRenderTargetView* GetCurrentRenderTargetView();

		const uint32 GetCurrentBackBufferIndex() const { return m_currentBackBufferIndex; }
		const uint32 GetWidth() const;
		const uint32 GetHeight() const;
		const Math::Vectoru2& GetDimensions() const;

		Platform::WindowHandle GetWindowHandle() const;

		bool GetIsTearingSupported() const;

		ERHIFormat GetRenderTargetFormat() const;

	protected:

	private:
		IRHISwapchain(uint32 width, uint32 height, bool isTearingSupported);
		IRHISwapchain(const IRHISwapchain&) = delete;
		IRHISwapchain(IRHISwapchain&&) = delete;
		IRHISwapchain& operator=(const IRHISwapchain&) = delete;
		IRHISwapchain& operator=(IRHISwapchain&&) = delete;
		virtual ~IRHISwapchain() = default;

		ERHIFormat m_renderTargetFormat = ERHIFormat::INVALID;

		uint32 m_currentBackBufferIndex = 0;
		Math::Vectoru2 m_dimensions;

		uint32 m_width = 0;
		uint32 m_height = 0;

		bool m_isTearingSupported = false;

		Platform::WindowHandle m_windowHandle;
	};

	/* Swapchain */
	template <IRHISwapchain::EFrameBuffering _F>
	class RHISwapchain : public IRHISwapchain
	{
	public:
		constexpr static uint8 GetNumBackBuffers() { return static_cast<uint8>(_F); }

		RHIResource* mp_backBufferResources[GetNumBackBuffers()];
		RHIRenderTargetView* mp_backBufferRTVs[GetNumBackBuffers()];

	public:
		
	};
}

#endif