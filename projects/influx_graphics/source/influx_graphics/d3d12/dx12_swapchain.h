#pragma once
#include "influx_graphics/swapchain.h"

struct IDXGISwapChain4;

namespace influx::graphics
{
	class dx12_resource;

	class dx12_swapchain final : public swapchain
	{
	public:
		dx12_swapchain(const swapchain_desc& desc, IDXGISwapChain4* swapchain4);

		virtual void present(const present_args& args) override;

		virtual resource* get_backbuffer_resource(uint8 at_index) const override;

		virtual uint8 get_current_backbuffer_index() const override;

	private:
		IDXGISwapChain4* mpdxgi_swapchain4;
		vector<dx12_resource*> mp_buffer_resources;

		void create_resources();
	};
}