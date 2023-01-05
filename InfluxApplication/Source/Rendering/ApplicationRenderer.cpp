#include "ApplicationRenderer.h"

#include "InfluxGraphics/RHIDevice.h"

namespace Influx::Application
{
	ApplicationRenderer::ApplicationRenderer(IRenderer::RHIDevicePtr device, const WindowInfo& windowInfo)
		: IRenderer(device)
		, m_initialWindowInfo{windowInfo}
	{
		Initialize(device);
	}

	ApplicationRenderer::~ApplicationRenderer()
	{
		Cleanup(mp_deviceRef);
	}

	void ApplicationRenderer::Initialize(const IRenderer::RHIDevicePtr devicePtr)
	{
		mp_commandQueue = devicePtr->CreateCommandQueue(Graphics::ERHICommandQueueType::Graphics);
		mp_swapChain	= devicePtr->CreateSwapchain(m_initialWindowInfo.WindowDimensions, m_initialWindowInfo.WindowHandle, mp_commandQueue);

		mp_rtvDescriptorHeap = devicePtr->CreateDescriptorHeap(Graphics::ERHIDescriptorType::RTV, 3u, 0);
	}

	void ApplicationRenderer::OnRender() const
	{
		
	}

	void ApplicationRenderer::Cleanup(const IRenderer::RHIDevicePtr devicePtr)
	{

	}
}
