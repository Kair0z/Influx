#pragma once
#include "influx_graphics/swapchain.h"

namespace influx::graphics
{
	class dx12_swapchain final : public swapchain
	{
	public:
		dx12_swapchain(const swapchain_desc& desc, IDXGISwapChain4* swapchain4)
			: swapchain(desc)
		{
			mp_native = mpdxgi_swapchain4 = swapchain4;
		}

		virtual void present(const present_args& args) override
		{
			mpdxgi_swapchain4->Present(args.m_vsync ? 1 : 0, 0u);
		}

	private:
		IDXGISwapChain4* mpdxgi_swapchain4;
	};
}