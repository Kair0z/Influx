#pragma once

#include "GraphicsAPI.h"
#include "RHIPipeline.h"
#include "RHIResource.h"
#include "RHIRenderPass.h"

namespace Influx
{
	class RenderInterface
	{
		virtual void InitializeRHI(const Graphics::GraphicsAPI* gfxApi) = 0;
		virtual void OnRender(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture) = 0;
	};

	class Renderer final : public RenderInterface
	{
	public:
		Renderer() = default;
		virtual void InitializeRHI(const Graphics::GraphicsAPI* gfxApi) override final;
		virtual void OnRender(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture) override final;

		~Renderer();

	private:
		void CreateRenderPasses(const Graphics::GraphicsAPI* gfxApi);
		void CreatePipelineObjects(const Graphics::GraphicsAPI* gfxApi);
		void CreateShaders(const Graphics::GraphicsAPI* gfxApi);

	protected:
		Graphics::RHIGraphicsPipelineLayout* GfxPipelineLayout;
		Graphics::RHIGraphicsPipeline* GfxPipeline;
		Graphics::RHIViewport GfxViewport;
		Graphics::RHIScissorRect GfxScissorRect;

		Graphics::RHIVertexBuffer* GfxSceneVertexBuffer;

		Graphics::RHIShader* GfxVertexShader;
		Graphics::RHIShader* GfxPixelShader;

		Graphics::RHIRenderPass* mpGfxRenderPass;
	};
}


