#pragma once

#ifndef __GR_RHI_DEVICE_H_
#define __GR_RHI_DEVICE_H_

#include "RHITypes.h"
#include "Core/Pointer.h"
#include "Core/Platform/Platform.h"

namespace Influx::Graphics
{
	class RHICommandList;
	class RHIDescriptorHeap;
	class RHISwapchain;
	class RHICommandQueue;
	class RHIRenderTargetView;
	class RHIResource;

	class RHIDevice
	{
	protected:
		using CommandQueuePtr		= Ptr<RHICommandQueue>;
		using SwapchainPtr			= Ptr<RHISwapchain>;
		using DescriptorHeapPtr		= Ptr<RHIDescriptorHeap>;
		using RenderTargetViewPtr	= Ptr<RHIRenderTargetView>;
		using ResourcePtr			= Ptr<RHIResource>;

	public:
		/* Creating API objects & Resources */
		virtual CommandQueuePtr CreateCommandQueue(const ERHICommandQueueType type) const = 0;

		virtual SwapchainPtr CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, CommandQueuePtr commandQueue) const = 0;

		virtual DescriptorHeapPtr CreateDescriptorHeap(const ERHIResourceViewType type, uint32 numDescriptors, bool isShaderVisible) const = 0;

		virtual RenderTargetViewPtr CreateRenderTargetView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const = 0;

		virtual ResourcePtr CreateResource(const ERHIResourceState initialState) const = 0;
		virtual ResourcePtr CreateTextureResource(const ERHIResourceState initialState, const ERHIFormat format, const Math::Vectoru2& dimensions, const uint16 numMips) const = 0;

		/* Debug Layer*/
		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) = 0;

		bool GetIsDebugLayerEnabled() const;
		
	protected:
		RHIDevice() = default;
		RHIDevice(const RHIDevice&) = delete;
		RHIDevice(RHIDevice&&) = delete;
		RHIDevice& operator=(const RHIDevice&) = delete;
		RHIDevice& operator=(RHIDevice&&) = delete;
		virtual ~RHIDevice() = default;

	private:
		bool m_isDebugLayerEnabled = false;
	};
}

#endif


