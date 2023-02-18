
#include "InfluxRenderer/IRenderer.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"
#include "InfluxGraphics/RHIResource.h"
#include "InfluxGraphics/RHIRootSignature.h"
#include "InfluxGraphics/RHIPipeline.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"
#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"

#include "Core/Platform/WindowsPlatform.h"

#include "Renderer/GUIRenderer.h"
#include "Widgets/ViewportWidget.h"

using namespace Influx;

namespace Settings
{
	bool g_vsync = true;
}

int main()
{
	// [Create Window]
	Platform::WindowSettings windowSettings({ 640u, 480u }, "Flux Renderer");
	Platform::WindowHandle wndHandle = Platform::CreateWindow(windowSettings, true);
	
	// [Create Graphics Interface + Swapchain]
	Graphics::D3D12Device* device		= new Graphics::D3D12Device(true);
	Graphics::RHICommandQueue* cmdQueue = device->GetGlobalGraphicsCommandQueue();

	// [Create Window Swapchain]
	Graphics::RHISwapchain* swapchain	= device->CreateSwapchain({ windowSettings.Width, windowSettings.Heigth }, wndHandle, cmdQueue);

	// [Create InputLayout & RenderPipeline]
	Graphics::RHIRootSignature* rootSignature = device->CreateGraphicsRootSignature();

	Graphics::RHIPipelineDescription pipelineDesc{};
	pipelineDesc.BlendState;
	pipelineDesc.DepthStencilState;
	pipelineDesc.VS;
	pipelineDesc.PS;
	pipelineDesc.PrimitiveTopologyType;
	pipelineDesc.RasterizerState;
	pipelineDesc.RenderTargets[0].bEnableBlend = false;
	pipelineDesc.RenderTargets[0].bEnableLogicOp = false;
	pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;

	Graphics::RHIPipeline* graphicsPipeline = device->CreateGraphicsPipeline(pipelineDesc, rootSignature);

	{
		// [Create Separate Scene Buffer]
		Graphics::RHIResource* sceneColourBuffer = device->CreateTextureResource(
			Graphics::ERHIResourceState::RenderTarget, Graphics::ERHIFormat::RGBA_8_Unorm, Math::Vectoru2{windowSettings.Width, windowSettings.Heigth}, 1u);
		
		Graphics::RHIRenderTargetView*	sceneColourRenderTarget = sceneColourBuffer->CreateRenderTargetView(device);

		while (Platform::PollWindowEvents(wndHandle))
		{
			Graphics::RHIResource* swapchainCurrentBuffer		= swapchain->GetCurrentBackBufferResource();
			Graphics::RHIRenderTargetView* swapchainCurrentRtv	= swapchain->GetCurrentRenderTargetView();
			Graphics::RHICommandList* cmdList = cmdQueue->SetupNewCommandList(device);
			
			// Bind global device descriptorHeaps...
			cmdList->BindDescriptorheap(device->GetResourceDescriptorHeap());

			// Clear Scene colour:
			cmdList->ClearRTV(sceneColourRenderTarget, {1.0f, 0.0f, 0.0f, 1.0f });
			
			cmdList->CopyResource(sceneColourBuffer, swapchainCurrentBuffer);

			cmdList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::Present);
			cmdQueue->ExecuteCommmandList(cmdList);
			
			swapchain->Present(cmdQueue, Settings::g_vsync);
		}

		//guiRenderer.Cleanup(device);
	}
}