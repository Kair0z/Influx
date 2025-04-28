#pragma once
#include "influx_graphics/swapchain.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct IDXGISwapChain4;

namespace influx::graphics
{
	class dx12_swapchain final : public swapchain
	{
	private:
		IDXGISwapChain4* mpdxgi_swapchain4;

	private:
		dx12_swapchain(
			const swapchain_desc& desc,
			const swapchain_dependencies& swapchain_dependencies,
			IDXGISwapChain4* swapchain4);

		virtual ~dx12_swapchain();
		virtual void release_impl(device*) override;

		// swapchain interface begin
		virtual result<>					present(const present_args& args) override;
		virtual result<>					acquire_backbuffer() override;
		virtual uint8						get_current_backbuffer_index() override;
		virtual result<vector<resource*>>	create_resources(device*) override;
		virtual result<>					resize_impl(const math::vectoru2& old_dim, const math::vectoru2& new_dim) override;
		virtual result<>					destroy_resources(device*) override;
		// swapchain interface end

		friend class dx12_device;
	};
}