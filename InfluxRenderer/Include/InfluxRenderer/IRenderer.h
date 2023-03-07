#pragma once

#ifndef __RENDERER_IRENDERER_H_
#define __RENDERER_IRENDERER_H_

#include "InfluxGraphics/RHITypes.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHICommandList;
	class RHIGraphicsPipelineLayout;
	class RHIGraphicsPipeline;
	class RHIDescriptorHeap;
	class RHITexture;
	class RHIResource;
}

namespace Influx::Renderer
{
	class RenderContext;

	class IRenderer
	{
	protected:
		IRenderer() = default;
		virtual ~IRenderer() = default;
		friend class RootRenderer;

	private:
		/* After initializing the RHI API Device */
		/* Here you initialize / create your RHI objects */
		virtual void OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) {}

		/* */
		virtual void OnBuildRenderCommandList(
			const RenderContext& context, 
			Graphics::RHICommandList* cmdList) {};

		/* */
		virtual void OnWindowResize(const Renderer::RenderContext& context, 
			const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize) {};

		/* Before cleaning up the RHI API Device */
		/* Here you Release your RHI objects */
		virtual void OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) {};
	};
}

#endif