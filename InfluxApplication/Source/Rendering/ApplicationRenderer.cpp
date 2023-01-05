#include "ApplicationRenderer.h"

#include "InfluxGraphics/RHIDevice.h"
#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHICommandList.h"
#include "InfluxGraphics/RHISwapchain.h"

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
		Graphics::RHICommandList* cmdList = mp_commandQueue->SetupNewCommandList(GetDeviceReference());
		
		cmdList->TransitionResource(mp_swapChain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::RenderTarget);

		cmdList->ClearRTV(mp_swapChain->GetCurrentRenderTargetView(), { 1.0f, 0.0f, 0.0f, 1.0f });

		cmdList->TransitionResource(mp_swapChain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::Present);

		mp_commandQueue->ExecuteCommmandList(cmdList);
		mp_swapChain->Present(mp_commandQueue, true);
	}

	void ApplicationRenderer::Cleanup(const IRenderer::RHIDevicePtr devicePtr)
	{

	}
}
