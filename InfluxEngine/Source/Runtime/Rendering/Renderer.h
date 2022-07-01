#pragma once

#include "GraphicsAPI.h"
#include "RHIPipeline.h"
#include "RHIResource.h"

namespace Influx
{
	class Renderer final
	{
	public:
		static Ptr<Renderer> Create(const Graphics::GraphicsAPI* gfxApi);

		virtual void Render(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture);

		~Renderer();

	private:
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

		Renderer() = default;
	};
}


