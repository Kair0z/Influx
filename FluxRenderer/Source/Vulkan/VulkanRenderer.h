#pragma once

#include "../Renderer/IFluxRenderer.h"

namespace Influx
{
	class VulkanRenderer : public IFluxRenderer
	{
	public:
		VulkanRenderer() = default;

	private:
		virtual void BuildRenderWork(Platform::WindowHandle windowHandle) override final;

		virtual void PresentToWindow(Platform::WindowHandle windowHandle) override final;

		void WaitForPreviousFrame();

	private:
		void Initialize();
	};
}


