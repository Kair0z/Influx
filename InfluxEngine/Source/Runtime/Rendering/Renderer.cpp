#include "pch.h"
#include "Renderer.h"

#include "Geometry/Vertex.h"

Influx::Vertex gTriangleVertices[3] =
{
	Influx::Vertex{{-0.25f, -0.15f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}},
	Influx::Vertex{{0.0f, 0.25f, 0.0f}, {0.0f, 1.0f, 0.0f}, {}},
	Influx::Vertex{{0.25f, -0.15f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}}
};

namespace Influx
{
	void Renderer::InitializeRHI(const Graphics::GraphicsAPI* gfxApi)
	{
		CreateShaders(gfxApi);
		CreatePipelineObjects(gfxApi);

		GfxSceneVertexBuffer = gfxApi->CreateVertexBuffer(&gTriangleVertices[0].Position[0], sizeof(gTriangleVertices), sizeof(Vertex));
	}

	void Renderer::OnRender(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture)
	{
		GfxViewport = Graphics::RHIViewport(gameRenderTexture->GetWidth(), gameRenderTexture->GetHeight());
		GfxScissorRect = Graphics::RHIScissorRect(gameRenderTexture->GetWidth(), gameRenderTexture->GetHeight());

		cmdList->BindPipelineLayout(GfxPipelineLayout);
		cmdList->BindPipelineState(GfxPipeline);
		cmdList->BindViewports(GfxViewport);
		cmdList->BindScissorRect(GfxScissorRect);

		cmdList->BindVertexBuffer(GfxSceneVertexBuffer);
		cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);
		cmdList->BindRenderTarget(gameRenderTexture->GetRenderTargetView());
		cmdList->DrawInstanced(3, 1);
	}

	void Renderer::CreatePipelineObjects(const Graphics::GraphicsAPI* gfxApi)
	{
		// Create Pipeline Objects & Pipeline Layout:
		Graphics::RHIGraphicsPipelineLayoutDescription pipelineLayoutDescription{};
		pipelineLayoutDescription.LayoutBindings;
		GfxPipelineLayout = gfxApi->CreateGraphicsPipelineLayout(pipelineLayoutDescription);

		Graphics::RHIGraphicsPipelineDescription pipelineDescription{};
		pipelineDescription.bAntialiasedLineEnable = false;
		pipelineDescription.bConservativeRaster = false;
		pipelineDescription.bDepthEnabled = false;
		pipelineDescription.bRasterDepthClipEnable = false;
		pipelineDescription.bStencilEnabled = false;
		pipelineDescription.DSVFormat = Graphics::ERHIFormat::D_32_Float;
		pipelineDescription.RTVFormats = {Graphics::ERHIFormat::RGBA_8_Unorm};
		pipelineDescription.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;
		pipelineDescription.RasterCullMode = Graphics::ERHICullMode::BackFaceCull;
		pipelineDescription.RasterFillMode = Graphics::ERHIFillMode::Solid;
		pipelineDescription.RasterDepthBias;
		pipelineDescription.RasterMaxDepthBias;
		pipelineDescription.VertexShader = GfxVertexShader;
		pipelineDescription.PixelShader = GfxPixelShader;
		GfxPipeline = gfxApi->CreateGraphicsPipeline(pipelineDescription, GfxPipelineLayout);
	}

	void Renderer::CreateShaders(const Graphics::GraphicsAPI* gfxApi)
	{
		GfxVertexShader = gfxApi->CreateRHIShader(L"Source/Shaders/DefaultLitShader.hlsl", "VertexMain", Graphics::ERHIShaderType::VertexShader, Graphics::ERHIShaderModel::SM_5_0);
		GfxPixelShader = gfxApi->CreateRHIShader(L"Source/Shaders/DefaultLitShader.hlsl", "PixelMain", Graphics::ERHIShaderType::PixelShader, Graphics::ERHIShaderModel::SM_5_0);
	}

	Renderer::~Renderer()
	{
		delete GfxVertexShader;
		delete GfxPixelShader;
		delete GfxPipelineLayout;
		delete GfxPipeline;
		delete GfxSceneVertexBuffer;
	}
	
}