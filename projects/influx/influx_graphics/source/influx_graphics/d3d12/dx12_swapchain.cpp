#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_swapchain::dx12_swapchain(const swapchain_desc& desc, 
		const swapchain_dependencies& swapchain_dependencies, 
		IDXGISwapChain4* swapchain4)
		: swapchain(desc, swapchain_dependencies)
	{
		mp_native = mpdxgi_swapchain4 = swapchain4;
		set_releasable(mpdxgi_swapchain4);

		mp_buffer_resources = create_resources();
	}

	dx12_swapchain::~dx12_swapchain()
	{
	}

	void dx12_swapchain::present(const present_args& args)
	{
		mpdxgi_swapchain4->Present(args.m_vsync ? 1 : 0, 0u);
	}

	uint8 dx12_swapchain::acquire_backbuffer()
	{
		const uint8 index = mpdxgi_swapchain4->GetCurrentBackBufferIndex();
		update_backbuffer_index(index);
		return index;
	}

	vector<resource*> dx12_swapchain::create_resources()
	{
		vector<resource*> resources{};
		for (uint8 i = 0u; i < get_num_backbuffers(); ++i)
		{
			ID3D12Resource* dxresource = nullptr;
			mpdxgi_swapchain4->GetBuffer(i, IID_PPV_ARGS(&dxresource));

			tex2D_desc desc{};
			desc.m_format = get_format();
			desc.m_dimensions = get_dimensions();

			resources.push_back(new dx12_resource(dxresource, desc));
		}
		return resources;
	}
}