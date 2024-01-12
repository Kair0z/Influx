#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_resource.h"

namespace influx::graphics
{
	dx12_swapchain::dx12_swapchain(const swapchain_desc& desc, IDXGISwapChain4* swapchain4)
		: swapchain(desc)
	{
		mp_native = mpdxgi_swapchain4 = swapchain4;

		create_resources();
	}

	void dx12_swapchain::present(const present_args& args)
	{
		mpdxgi_swapchain4->Present(args.m_vsync ? 1 : 0, 0u);
	}

	resource* dx12_swapchain::get_backbuffer_resource(uint8 at_index) const
	{
		return mp_buffer_resources[at_index];
	}

	uint8 dx12_swapchain::get_current_backbuffer_index() const
	{
		return mpdxgi_swapchain4->GetCurrentBackBufferIndex();
	}

	void dx12_swapchain::create_resources()
	{
		// delete & clear previous
		for (dx12_resource* res : mp_buffer_resources)
		{
			delete res;
			res = nullptr;
		}
		mp_buffer_resources.clear();

		// recreate
		for (uint8 i = 0u; i < get_num_backbuffers(); ++i)
		{
			ID3D12Resource* dxresource = nullptr;
			mpdxgi_swapchain4->GetBuffer(i, IID_PPV_ARGS(&dxresource));

			// this is a bit cringe...
			tex2D_desc desc{};
			desc.m_format = get_desc().m_format;
			desc.m_dimensions = get_desc().m_dimensions;

			mp_buffer_resources.push_back(new dx12_resource(dxresource, desc));
		}
	}
}