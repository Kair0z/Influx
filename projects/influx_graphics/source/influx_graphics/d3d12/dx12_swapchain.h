#pragma once
#include "influx_graphics/swapchain.h"

struct IDXGISwapChain4;

namespace influx::graphics
{
	class dx12_swapchain final : public swapchain
	{
		// acquires the next available backbuffer (and returns the index)
		virtual uint8 acquire_backbuffer() override;

		virtual vector<resource*> create_resources() override;

	public:
		dx12_swapchain(
			const swapchain_desc& desc,
			const swapchain_dependencies& swapchain_dependencies,
			IDXGISwapChain4* swapchain4);

		virtual void present(const present_args& args) override;

	private:
		IDXGISwapChain4* mpdxgi_swapchain4;
	};
}