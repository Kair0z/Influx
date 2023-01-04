#pragma once

#ifndef __GR_RHI_DEVICE_H_
#define __GR_RHI_DEVICE_H_

#include "RHITypes.h"
#include "Core/Platform/Platform.h"

namespace Influx::Graphics
{
	class RHICommandList;
	class RHIDescriptorHeap;
	class RHISwapchain;
	class RHICommandQueue;
	class RHIResource;

	class RHIDevice
	{
	public:
		/* Creating API objects & Resources */
		virtual RHICommandQueue* CreateCommandQueue(const ERHICommandQueueType type) const = 0;
		virtual RHISwapchain* CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, RHICommandQueue* commandQueue) const = 0;
		virtual RHIDescriptorHeap* CreateDescriptorHeap(const ERHIDescriptorType type, uint32 numDescriptors, bool isShaderVisible) const = 0;

		virtual RHIResource* CreateResource() const = 0;

		/* Debug Layer*/
		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) = 0;
		bool GetIsDebugLayerEnabled() const
		{
			return m_isDebugLayerEnabled;
		}

	protected:
		RHIDevice() = default;
		RHIDevice(const RHIDevice&) = delete;
		RHIDevice(RHIDevice&&) = delete;
		RHIDevice& operator=(const RHIDevice&) = delete;
		RHIDevice& operator=(RHIDevice&&) = delete;
		virtual ~RHIDevice();

	private:
		bool m_isDebugLayerEnabled = false;
	};
}

#endif


