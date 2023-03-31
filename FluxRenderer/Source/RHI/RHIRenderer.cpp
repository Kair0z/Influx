#include "RHIRenderer.h"

#include "InfluxGraphics/RHI.h"

// Todo... Get rid of these ugly includes :(
#include "InfluxGraphics/D3D12/D3D12Device.h"			// <- We need this to specify Graphics-API...
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"	// <- We need this because 'GetResourceDescriptorHeap()' is D3D12Device specific...
#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"		// <- We need this because 'GetGlobalGraphicsCommandQueue()' is D3D12Device specific...

#include "Core/Platform/WindowsPlatform.h"

namespace Influx
{
	void RHIRenderer::BuildRenderWork(Platform::WindowHandle windowHandle)
	{
		// [Setup Graphics]
		InitializeDevice();
		InitializeCommandQueue();

		// Setup GraphicsPipeline & Layout
		InitializeRenderPipeline();

		// Setup Vertex & indexbuffer
		UpdateSceneBufferData();


		Platform::WindowSettings settings = Platform::GetWindowSettings(windowHandle);
		Graphics::RHIResource* swapchainCurrentBuffer		= mp_swapchain->GetCurrentBackBufferResource();
		Graphics::RHIRenderTargetView* swapchainCurrentRtv	= mp_swapchain->GetCurrentRenderTargetView();
		Graphics::RHIViewport viewport = { (float)settings.Width, (float)settings.Heigth };
		Graphics::RHIScissorRect scissorRect = { (uint32)settings.Width, (uint32)settings.Heigth };

		// New command list...
		mp_commandList = mp_commandQueue->SetupNewCommandList(mp_device);


		// Bind global device descriptorHeaps...
		mp_commandList->BindDescriptorheap(mp_device->GetResourceDescriptorHeap());


		// Clear Scene colour:
		mp_commandList->ClearRTV(mp_sceneColourRtv, { 1.0f, 0.0f, 0.0f, 1.0f });

		// Draw Triangle:
		{
			mp_commandList->BindPipelineLayout(mp_pipelineLayout);
			mp_commandList->BindPipelineState(mp_pipeline);
			mp_commandList->BindRenderTarget(mp_sceneColourRtv);
			mp_commandList->BindViewports(viewport);
			mp_commandList->BindScissorRect(scissorRect);

			mp_commandList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);
			mp_commandList->BindIndexBuffer(mp_indexBuffer, mp_indexBuffer->GetNumBytes());
			mp_commandList->BindVertexBuffer(mp_vertexBuffer, mp_vertexBuffer->GetNumBytes(), sizeof(Scene::Mesh::Vertex));
			
			const uint32 numIndices = static_cast<uint32>(mp_indexBuffer->GetNumBytes()) / sizeof(uint32);
			mp_commandList->DrawIndexedInstanced(numIndices, 1u, 0u, 0u, 0u);
		}

		// Copy Scene Colour -> swapchainBackbuffer
		mp_commandList->CopyResource(mp_sceneColour, swapchainCurrentBuffer);
		mp_commandList->TransitionResource(swapchainCurrentBuffer, Graphics::ERHIResourceState::Present);
	}

	void RHIRenderer::PresentToWindow(Platform::WindowHandle windowHandle)
	{
		InitializeSwapchain(windowHandle);

		mp_commandQueue->ExecuteCommmandList(mp_commandList);

		const bool vsync = true;
		mp_swapchain->Present(mp_commandQueue, vsync);
	}

	void RHIRenderer::InitializeDevice()
	{
		if (mp_device == nullptr)
		{
			mp_device = new Graphics::D3D12Device(true);
		}
	}

	void RHIRenderer::InitializeCommandQueue()
	{
		if (mp_device != nullptr && mp_commandQueue == nullptr)
		{
			mp_commandQueue = mp_device->GetGlobalGraphicsCommandQueue();
		}
	}

	void RHIRenderer::InitializeSwapchain(Platform::WindowHandle windowHandle)
	{
		Platform::WindowSettings settings = Platform::GetWindowSettings(windowHandle);

		if (mp_device != nullptr && mp_swapchain == nullptr && mp_commandQueue != nullptr)
		{
			mp_swapchain = mp_device->CreateSwapchain({ settings.Width, settings.Heigth }, windowHandle, mp_commandQueue);
		}

		// Create scene resource buffer.
		if (mp_swapchain && mp_sceneColour == nullptr)
		{
			mp_sceneColour = mp_device->CreateTextureResource(Graphics::ERHIResourceState::RenderTarget, Graphics::ERHIFormat::RGBA_8_Unorm, 
				Math::Vectoru2{ settings.Width, settings.Heigth }, 1u);

			mp_sceneColourRtv = mp_sceneColour->CreateRenderTargetView(mp_device);
		}
	}

	void RHIRenderer::InitializeRenderPipeline()
	{
		// [Create InputLayout & RenderPipeline]
		Graphics::RHIGraphicsPipelineLayout* pipelineLayout = mp_device->CreateGraphicsPipelineLayout(); // For now, default no-inputs pipeline-layout...
		Graphics::RHIGraphicsPipeline* graphicsPipeline = nullptr;
		{
			Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR", 0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });

			pipelineDesc.VS = GetMaterial().VertexShader;
			pipelineDesc.PS = GetMaterial().PixelShader;

			pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

			pipelineDesc.BlendState = Graphics::RHIBlendState::GetDefault();
			pipelineDesc.RasterizerState = Graphics::RHIRasterizerState::GetDefault();
			pipelineDesc.DepthStencilState = Graphics::RHIDepthStencilState::GetDefault();

			pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;

			graphicsPipeline = mp_device->CreateGraphicsPipeline(pipelineDesc, pipelineLayout);
		}
	}

	void RHIRenderer::UpdateSceneBufferData()
	{
		if (mp_vertexBuffer != nullptr || mp_indexBuffer != nullptr)
		{
			//...
			return;
		}

		Graphics::RHIResource* vertexBuffer = mp_device->CreateVertexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, GetVertexBufferSize());
		Graphics::RHIResource* indexBuffer	= mp_device->CreateIndexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, GetIndexBufferSize());
		
		Vector<Scene::Mesh::Vertex> allVertices{};
		Vector<Scene::Mesh::Index> allIndices{};

		for (const Scene::Mesh& mesh : GetMeshes())
		{
			for (const Scene::Mesh::Vertex& vertex : mesh.GetVertices())
				allVertices.push_back(vertex);

			for (const Scene::Mesh::Index& index : mesh.GetIndices())
				allIndices.push_back(index);
		}
		
		// Copy data to GPU buffers...
		vertexBuffer->ScopedMap([this, allVertices](void* cpuHandle) 
			{
				memcpy(cpuHandle, allVertices.data(), allVertices.size() * sizeof(Scene::Mesh::Vertex));
			});
		indexBuffer->ScopedMap([this, allIndices](void* cpuHandle)
			{
				memcpy(cpuHandle, allIndices.data(), allIndices.size() * sizeof(Scene::Mesh::Index));
			});
	}
}


