#pragma once

#include "../Renderer/IFluxRenderer.h"

namespace Influx
{
	class Dx12Renderer final : public IFluxRenderer
	{
	public:
		Dx12Renderer() = default;

	private:
		virtual void BuildRenderWork(Platform::WindowHandle windowHandle) override final;

		virtual void PresentToWindow(Platform::WindowHandle windowHandle) override final;
	};
}


