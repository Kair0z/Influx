#include "renderer_pch.h"

#include "InfluxRenderer/RootRenderer.h"
#include "InfluxRenderer/Renderers/SceneRenderer.h"

#include "InfluxGraphics/RHI.h"

#include "Core/Geometry/Vertex.h"

#include <d3dcompiler.h>

namespace Influx::Renderer
{
	void SceneRenderer::SetSceneToRender(const Influx::Scene::Scene& scene)
	{
		m_scene = scene;
	}

	const Influx::Scene::Scene& SceneRenderer::GetSceneToRender() const
	{
		return m_scene;
	}

	void SceneRenderer::OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{
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
			result = ::D3DCompileFromFile(L"D:/Git/Influx/Resources/Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
			result = ::D3DCompileFromFile(L"D:/Git/Influx/Resources/Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

			for (uint32 i = 0; i < vertexShader->GetBufferSize(); ++i)
			{
				m_compiledVertexShader.push_back(reinterpret_cast<uint8*>(vertexShader->GetBufferPointer())[i]);
			}

			for (uint32 i = 0; i < pixelShader->GetBufferSize(); ++i)
			{
				m_compiledPixelShader.push_back(reinterpret_cast<uint8*>(pixelShader->GetBufferPointer())[i]);
			}
		}
	}

	void SceneRenderer::OnBuildRenderCommandList(const Renderer::RenderContext& context, Graphics::RHICommandList* cmdList)
	{
		Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
		{
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
			pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR", 0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });

			pipelineDesc.VS = m_compiledVertexShader;
			pipelineDesc.PS = m_compiledPixelShader;

			pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

			pipelineDesc.BlendState = Graphics::RHIBlendState::GetDefault();
			pipelineDesc.RasterizerState = Graphics::RHIRasterizerState::GetDefault();
			pipelineDesc.DepthStencilState = Graphics::RHIDepthStencilState::GetDefault();

			pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		}

		Graphics::RHIGraphicsPipelineLayoutDescription layoutDesc{};

		Graphics::RHITextureDesc sceneColourDesc{};
		sceneColourDesc.Dimensions = { context.GetSwapchain()->GetWidth(), context.GetSwapchain()->GetHeight() };
		sceneColourDesc.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		sceneColourDesc.NumMips = 1;

		Graphics::RHIViewport viewport{};
		Graphics::RHIScissorRect scissorRect{};

		mp_pipelineLayout	= context.GetAndOrCreateGraphicsPipelineLayout(layoutDesc);
		mp_pipeline			= context.GetAndOrCreateGraphicsPipeline(pipelineDesc, layoutDesc);

		mp_sceneColourTexture = context.GetAndOrCreateTexture("SceneColour", sceneColourDesc);
		Graphics::RHIRenderTargetView* sceneColourRTV = mp_sceneColourTexture->GetAndOrCreateRenderTargetView(context.GetDevice());

		// Vertex data:
		Vector<Math::Vertex> vertices{};
		Vector<uint64> indices{};
		{
			const Vector<Scene::Mesh>& meshes = m_scene.GetMeshes();

			for (const Scene::Mesh& mesh : meshes)
			{
				for (const Math::Vertex& vertex : mesh.GetVertices())
				{
					vertices.push_back(vertex);
				}

				for (const Scene::Mesh::Index& index : mesh.GetIndices())
				{
					indices.push_back(index);
				}
			}
		}

		const uint64 vertexSize			= sizeof(Influx::Math::Vertex);
		const uint64 numVertices		= vertices.size();
		const uint64 numIndices			= indices.size();
		const uint64 vertexBufferSize	= numVertices * vertexSize;
		const uint64 indexBufferSize	= numIndices * sizeof(uint64);

		Graphics::RHIResource* vertexBuffer = nullptr;
		if (vertexBufferSize > 0)
		{
			vertexBuffer = context.GetDevice()->CreateVertexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, vertexBufferSize);
			vertexBuffer->ScopedMap([&vertices, vertexBufferSize](void* cpuHandle) // Copy data to GPU buffer...
			{
				memcpy(cpuHandle, vertices.data(), vertexBufferSize);
			});
		}
		
		Graphics::RHIResource* indexBuffer = nullptr;
		if (indexBufferSize > 0)
		{
			indexBuffer = context.GetDevice()->CreateIndexBufferResource(Graphics::ERHIResourceState::GenericRead, Graphics::ERHIFormat::Unknown, indexBufferSize);
			indexBuffer->ScopedMap([&indices, indexBufferSize](void* cpuHandle)
			{
				memcpy(cpuHandle, indices.data(), indexBufferSize);
			});
		}

		// The actual rendering...
		{
			// Clear Scene colour:
			cmdList->ClearRTV(sceneColourRTV, { 1.0f, 0.0f, 0.0f, 1.0f });

			// Draw Triangle:
			{
				cmdList->BindPipelineLayout(mp_pipelineLayout);
				cmdList->BindPipelineState(mp_pipeline);

				cmdList->BindRenderTarget(sceneColourRTV);
				cmdList->BindViewports(viewport);
				cmdList->BindScissorRect(scissorRect);
				 
				cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);

				if (indexBuffer) cmdList->BindIndexBuffer(indexBuffer, indexBufferSize);
				if (vertexBuffer) cmdList->BindVertexBuffer(vertexBuffer, vertexBufferSize, vertexSize);
				if (indexBuffer) cmdList->DrawIndexedInstanced(numIndices, 1u, 0u, 0u, 0u);
			}

			context.CopyTextureIntoSwapchain(mp_sceneColourTexture, cmdList);
		}
	}

	void SceneRenderer::OnWindowResize(const Renderer::RenderContext& context, const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize)
	{

	}

	void SceneRenderer::OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{

	}
}

