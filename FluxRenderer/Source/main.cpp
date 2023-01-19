
#include "InfluxRenderer/IRenderer.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"
#include "Core/Platform/WindowsPlatform.h"

#include "Renderer/GUIRenderer.h"

using namespace Influx;

int main()
{
	Platform::WindowSettings windowSettings{};
	windowSettings.Width = 640u;
	windowSettings.Heigth = 480u;
	windowSettings.Name = "Flux Renderer";
	Platform::WindowHandle wndHandle = Platform::CreateWindow(windowSettings, true);

	Graphics::D3D12Device* device		= new Graphics::D3D12Device(true);
	Graphics::RHICommandQueue* cmdQueue = device->CreateCommandQueue(Graphics::ERHICommandQueueType::Graphics);
	Graphics::RHISwapchain* swapchain = device->CreateSwapchain({ windowSettings.Width, windowSettings.Heigth }, wndHandle, cmdQueue);

	const bool vsync = true;

	{
		GUI::GUIRenderer guiRenderer{};

		guiRenderer.Initialize(device);
		guiRenderer.AttachToRenderTarget(device, swapchain->GetCurrentRenderTargetView());

		while (Platform::PollWindowEvents(wndHandle))
		{
			Graphics::RHIResource* swapchainCurrentBuffer		= swapchain->GetCurrentBackBufferResource();
			Graphics::RHIRenderTargetView* swapchainCurrentRtv	= swapchain->GetCurrentRenderTargetView();

			Graphics::RHICommandList* cmdList = cmdQueue->SetupNewCommandList(device);

			cmdList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::RenderTarget);
			cmdList->ClearRTV(swapchainCurrentRtv, {1.0f, 0.0f, 0.0f, 1.0f});

			cmdList->BindRenderTarget(swapchainCurrentRtv);

			guiRenderer.Render(cmdList);

			cmdList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::Present);

			cmdQueue->ExecuteCommmandList(cmdList);

			swapchain->Present(cmdQueue, vsync);
		}

		guiRenderer.Cleanup(device);
	}
}