
#include "InfluxRenderer/IRenderer.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"
#include "InfluxGraphics/RHIResource.h"
#include "InfluxGraphics/RHIPipelineLayout.h"
#include "InfluxGraphics/RHIPipeline.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"
#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Core/Geometry/Vertex.h"

#include "Renderer/GUIRenderer.h"
#include "Widgets/ViewportWidget.h"

using namespace Influx;

namespace Settings
{
	bool g_vsync = true;
}

int main()
{
	// [Compile Shaders]
	Vector<byte> compiledVertexShader{};
	Vector<byte> compiledPixelShader{};
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ID3DBlob* vertexShader;
		ID3DBlob* pixelShader;

		HRESULT result{};
		result = D3DCompileFromFile(L"E:/Git/Influx/FluxRenderer/Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
		result = D3DCompileFromFile(L"E:/Git/Influx/FluxRenderer/Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

		for (uint32 i = 0; i < vertexShader->GetBufferSize(); ++i)
		{
			compiledVertexShader.push_back(reinterpret_cast<uint8*>(vertexShader->GetBufferPointer())[i]);
		}

		for (uint32 i = 0; i < pixelShader->GetBufferSize(); ++i)
		{
			compiledPixelShader.push_back(reinterpret_cast<uint8*>(pixelShader->GetBufferPointer())[i]);
		}
	}

	

	// [Create Window]
	Platform::WindowSettings windowSettings({ 640u, 480u }, "Flux Renderer");
	Platform::WindowHandle wndHandle = Platform::CreateWindow(windowSettings, true);
	const float aspectRatio = (float)windowSettings.Width / (float)windowSettings.Heigth;

	// [Create Graphics Interface + Swapchain]
	Graphics::D3D12Device* device		= new Graphics::D3D12Device(true);
	Graphics::RHICommandQueue* cmdQueue = device->GetGlobalGraphicsCommandQueue();

	// [Create Window Swapchain]
	Graphics::RHISwapchain* swapchain	= device->CreateSwapchain({ windowSettings.Width, windowSettings.Heigth }, wndHandle, cmdQueue);

	// [Create InputLayout & RenderPipeline]
	Graphics::RHIGraphicsPipelineLayout* pipelineLayout = device->CreateGraphicsPipelineLayout();
	Graphics::RHIGraphicsPipeline* graphicsPipeline = nullptr;
	{
		Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
		pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
		pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR", 0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });

		pipelineDesc.BlendState;
		pipelineDesc.DepthStencilState;
		pipelineDesc.VS = compiledVertexShader;
		pipelineDesc.PS = compiledPixelShader;
		pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

		pipelineDesc.RasterizerState	= Graphics::RHIRasterizerState::GetDefault();
		pipelineDesc.DepthStencilState	= Graphics::RHIDepthStencilState::GetDefault();

		pipelineDesc.RenderTargets[0].BlendDesc.bEnableBlend = false;
		pipelineDesc.RenderTargets[0].BlendDesc.bEnableLogicOp = false;
		pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;

		pipelineDesc.SampleCount = 1u;

		graphicsPipeline = device->CreateGraphicsPipeline(pipelineDesc, pipelineLayout);
	}
	
	// [Get Vertex Buffers]
	Influx::Math::Vertex triangle[3u]
	{
		{{0.0f, 0.25f * aspectRatio, 0.0f},		{1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.25f, -0.25f * aspectRatio, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.25f, -0.25f * aspectRatio, 0.0f},	{0.0f, 0.0f, 1.0f, 1.0f}}
	};

	const uint64 vertexBufferSize = sizeof(triangle);
	const uint64 vertexSize = sizeof(Influx::Math::Vertex);

	// [Create Triangle Buffer To Render]
	Graphics::RHIResource* vertexBuffer = device->CreateVertexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, vertexBufferSize);
	vertexBuffer->ScopedMap([&triangle](void* cpuHandle)
	{
		memcpy(cpuHandle, triangle, sizeof(triangle));
	});

	{
		// [Create Separate Scene Buffer]
		Graphics::RHIResource* sceneColourBuffer = device->CreateTextureResource(
			Graphics::ERHIResourceState::RenderTarget, Graphics::ERHIFormat::RGBA_8_Unorm, Math::Vectoru2{windowSettings.Width, windowSettings.Heigth}, 1u);
		
		Graphics::RHIRenderTargetView*	sceneColourRenderTarget = sceneColourBuffer->CreateRenderTargetView(device);
		Graphics::RHIViewport sceneColourViewport{(float)windowSettings.Width, (float)windowSettings.Heigth};
		Graphics::RHIScissorRect sceneColourScissorRect{(uint32)windowSettings.Width, (uint32)windowSettings.Heigth};

		while (Platform::PollWindowEvents(wndHandle))
		{
			Graphics::RHIResource* swapchainCurrentBuffer		= swapchain->GetCurrentBackBufferResource();
			Graphics::RHIRenderTargetView* swapchainCurrentRtv	= swapchain->GetCurrentRenderTargetView();
			Graphics::RHICommandList* cmdList = cmdQueue->SetupNewCommandList(device);
			
			// Bind global device descriptorHeaps...
			cmdList->BindDescriptorheap(device->GetResourceDescriptorHeap());

			cmdList->TransitionResource(sceneColourBuffer, Graphics::ERHIResourceState::RenderTarget);

			// Clear Scene colour:
			cmdList->ClearRTV(sceneColourRenderTarget, {1.0f, 0.0f, 0.0f, 1.0f });
			
			cmdList->BindPipelineLayout(pipelineLayout);
			cmdList->BindPipelineState(graphicsPipeline);
			cmdList->BindRenderTarget(sceneColourRenderTarget);
			cmdList->BindViewports(sceneColourViewport);
			cmdList->BindScissorRect(sceneColourScissorRect);

			cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);
			cmdList->BindVertexBuffer(vertexBuffer, vertexBufferSize, vertexSize);
			cmdList->DrawInstanced(3u, 1u);

			cmdList->CopyResource(sceneColourBuffer, swapchainCurrentBuffer);

			cmdList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::Present);
			cmdQueue->ExecuteCommmandList(cmdList);
			
			swapchain->Present(cmdQueue, Settings::g_vsync);
		}

		//guiRenderer.Cleanup(device);
	}
}