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

		virtual void Initialize() override final;

		virtual void Cleanup() override final;

	private:
		Graphics::RHISwapchainHandle m_swapchain;
	};
}


