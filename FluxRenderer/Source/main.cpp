
#include "InfluxGraphics/RHI.h"

#include "InfluxAssets/InfluxAssets.h"

// Todo... Get rid of these ugly includes :(
#include "InfluxGraphics/D3D12/D3D12Device.h"			// <- We need this to specify Graphics-API...
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"	// <- We need this because 'GetResourceDescriptorHeap()' is D3D12Device specific...
#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"		// <- We need this because 'GetGlobalGraphicsCommandQueue()' is D3D12Device specific...

#include "Core/Platform/WindowsPlatform.h"

#include "Core/Geometry/Vertex.h"

using namespace Influx;

namespace Settings
{
	bool g_vsync = true;

	const Influx::Math::Vectoru2 WindowDimensions{ 640u, 480u };
	const float AspectRatio = (float)WindowDimensions.x / (float)WindowDimensions.y;
}

struct Shaders final
{
	using ByteCode = Vector<byte>;

	ByteCode VertexShader;
	ByteCode PixelShader;
};

struct SceneData final
{
	using Index = uint64;
	using VertexBuffer = Vector<Math::Vertex>;
	using IndexBuffer = Vector<Index>;

	VertexBuffer VertexBuffer;
	IndexBuffer IndexBuffer;
};

int main()
{
	// [Compile Shaders]
	Shaders shaders{};
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
			shaders.VertexShader.push_back(reinterpret_cast<uint8*>(vertexShader->GetBufferPointer())[i]);
		}

		for (uint32 i = 0; i < pixelShader->GetBufferSize(); ++i)
		{
			shaders.PixelShader.push_back(reinterpret_cast<uint8*>(pixelShader->GetBufferPointer())[i]);
		}
	}

	// [Get Scene Data]
	SceneData sceneData{};
	if (Assets::Scene leblancScene{}; Assets::LoadScene("E:/Git/Influx/Resources/Meshes/box.fbx", leblancScene))
	{
		for (uint64 s = 0; s < leblancScene.Meshes.size(); ++s)
		{
			const Assets::Scene::Mesh& mesh = leblancScene.Meshes[s];
			for (uint64 i = 0; i < mesh.Vertices.size(); ++i)
			{
				Math::Vertex newVertex{};
				newVertex.Colour[i % 3]	= 1.0f;
				newVertex.Position		= mesh.Vertices[i].Position;
				sceneData.VertexBuffer.push_back(newVertex);
			}

			for (uint64 i = 0; i < mesh.Indices.size(); ++i)
			{
				sceneData.IndexBuffer.push_back(mesh.Indices[i]);
			}
		}
	}

	const uint64 vertexSize			= sizeof(Influx::Math::Vertex);
	const uint64 numVertices		= sceneData.VertexBuffer.size();
	const uint64 numIndices			= sceneData.IndexBuffer.size();
	const uint64 vertexBufferSize	= numVertices * vertexSize;
	const uint64 indexBufferSize	= numIndices * sizeof(uint64);
	
	// [Create Window]
	Platform::WindowSettings	windowSettings(Settings::WindowDimensions, "Flux Renderer");
	Platform::WindowHandle		wndHandle = Platform::CreateWindow(windowSettings, true);
	const float aspectRatio = (float)windowSettings.Width / (float)windowSettings.Heigth;

	// [Setup Graphics]
	Graphics::D3D12Device* device		= new Graphics::D3D12Device(true);
	Graphics::RHICommandQueue* cmdQueue = device->GetGlobalGraphicsCommandQueue();
	Graphics::RHISwapchain* swapchain	= device->CreateSwapchain({ windowSettings.Width, windowSettings.Heigth }, wndHandle, cmdQueue);

	// [Create InputLayout & RenderPipeline]
	Graphics::RHIGraphicsPipelineLayout* pipelineLayout = device->CreateGraphicsPipelineLayout(); // For now, default no-inputs pipeline-layout...
	Graphics::RHIGraphicsPipeline* graphicsPipeline		= nullptr;
	{
		Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
		pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
		pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR", 0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });

		pipelineDesc.VS = shaders.VertexShader;
		pipelineDesc.PS = shaders.PixelShader;

		pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

		pipelineDesc.BlendState			= Graphics::RHIBlendState::GetDefault();
		pipelineDesc.RasterizerState	= Graphics::RHIRasterizerState::GetDefault();
		pipelineDesc.DepthStencilState	= Graphics::RHIDepthStencilState::GetDefault();

		pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;

		graphicsPipeline = device->CreateGraphicsPipeline(pipelineDesc, pipelineLayout);
	}

	// [Create Triangle Buffer To Render]
	Graphics::RHIResource* vertexBuffer = device->CreateVertexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, vertexBufferSize);
	Graphics::RHIResource* indexBuffer = device->CreateIndexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, indexBufferSize);
	vertexBuffer->ScopedMap([&sceneData, vertexBufferSize](void* cpuHandle) // Copy data to GPU buffer...
	{
		memcpy(cpuHandle, sceneData.VertexBuffer.data(), vertexBufferSize);
	});
	indexBuffer->ScopedMap([&sceneData, indexBufferSize](void* cpuHandle)
	{
		memcpy(cpuHandle, sceneData.IndexBuffer.data(), indexBufferSize);
	});

	// [Create - Separate - Scene Colour Texture]
	struct SceneColour final
	{
		Graphics::RHIResource* Resource;
		Graphics::RHIRenderTargetView* RTV;
		Graphics::RHIViewport Viewport;
		Graphics::RHIScissorRect ScissorRect;
	} sceneColour;

	sceneColour.Resource = device->CreateTextureResource(Graphics::ERHIResourceState::RenderTarget, Graphics::ERHIFormat::RGBA_8_Unorm, Math::Vectoru2{ windowSettings.Width, windowSettings.Heigth }, 1u);
	sceneColour.RTV = sceneColour.Resource->CreateRenderTargetView(device);
	sceneColour.Viewport = { (float)windowSettings.Width, (float)windowSettings.Heigth };
	sceneColour.ScissorRect = { (uint32)windowSettings.Width, (uint32)windowSettings.Heigth };

	// [MAIN LOOP]
	while (Platform::PollWindowEvents(wndHandle))
	{
		Graphics::RHIResource*	swapchainCurrentBuffer = swapchain->GetCurrentBackBufferResource();
		Graphics::RHIRenderTargetView* swapchainCurrentRtv = swapchain->GetCurrentRenderTargetView();

		Graphics::RHICommandList* cmdList = cmdQueue->SetupNewCommandList(device);

		// Bind global device descriptorHeaps...
		cmdList->BindDescriptorheap(device->GetResourceDescriptorHeap());

		// Clear Scene colour:
		cmdList->ClearRTV(sceneColour.RTV, { 1.0f, 0.0f, 0.0f, 1.0f });

		// Draw Triangle:
		{
			cmdList->BindPipelineLayout(pipelineLayout);
			cmdList->BindPipelineState(graphicsPipeline);
			cmdList->BindRenderTarget(sceneColour.RTV);
			cmdList->BindViewports(sceneColour.Viewport);
			cmdList->BindScissorRect(sceneColour.ScissorRect);

			cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);
			cmdList->BindIndexBuffer(indexBuffer, indexBufferSize);
			cmdList->BindVertexBuffer(vertexBuffer, vertexBufferSize, vertexSize);
			cmdList->DrawIndexedInstanced(numIndices, 1u, 0u, 0u, 0u);
		}
		
		// Copy Scene Colour -> swapchainBackbuffer
		cmdList->CopyResource(sceneColour.Resource, swapchainCurrentBuffer);

		cmdList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::Present);

		cmdQueue->ExecuteCommmandList(cmdList);

		swapchain->Present(cmdQueue, Settings::g_vsync);
	}
}