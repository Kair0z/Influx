#pragma once

#include "../Renderer/IFluxRenderer.h"
#include "InfluxGraphics.h"

namespace influx
{
	class RHIRenderer final : public IFluxRenderer
	{
	private:
		virtual void RecordRenderCommands(platform::window_handle windowHandle) override final;

		virtual void PresentToWindow(platform::window_handle windowHandle) override final;

	public:
		RHIRenderer();
		virtual ~RHIRenderer();

	private:
		Graphics::RHISwapchainHandle m_swapchain;
	};
}


