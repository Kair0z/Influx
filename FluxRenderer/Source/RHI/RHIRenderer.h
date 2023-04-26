#pragma once

#include "../Renderer/IFluxRenderer.h"
#include "InfluxGraphics.h"

namespace Influx
{
	class RHIRenderer final : public IFluxRenderer
	{
	private:
		virtual void BuildRenderWork(Platform::WindowHandle windowHandle) override final;

		virtual void PresentToWindow(Platform::WindowHandle windowHandle) override final;

	public:
		RHIRenderer();
		virtual ~RHIRenderer();

	private:
		Graphics::RHISwapchainHandle m_swapchain;
	};
}


