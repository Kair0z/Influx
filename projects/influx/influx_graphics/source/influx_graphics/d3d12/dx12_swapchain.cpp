#include "graphics_pch.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "influx_graphics/d3d12/dx12_conversion.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_swapchain::dx12_swapchain(
		const swapchain_desc& desc, 
		const swapchain_dependencies& swapchain_dependencies, 
		IDXGISwapChain4* swapchain4)
		: swapchain(desc, swapchain_dependencies)
	{
		mp_native = mpdxgi_swapchain4 = swapchain4;

		get_resources() = create_resources(swapchain_dependencies.mp_device);
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

	void dx12_swapchain::release_impl(device*)
	{
		mpdxgi_swapchain4->Release();
	}

	vector<resource*> dx12_swapchain::create_resources(device* device)
	{
		const auto& dimensions = get_dimensions();
	
		vector<resource*> resources{};
		for (uint8 i = 0u; i < get_num_backbuffers(); ++i)
		{
			ID3D12Resource* dxresource = nullptr;
			mpdxgi_swapchain4->GetBuffer(i, IID_PPV_ARGS(&dxresource));

			tex2D_desc desc{};
			desc.m_format = get_format();
			desc.m_dimensions = dimensions;
			desc.m_init_state = e_resource_state::present;

			resource* new_resource = device->import_texture(dxresource, desc);
			resources.push_back(new_resource);
		}
		
		get_resources() = resources;
		return resources;
	}

	void dx12_swapchain::resize_impl(const math::vectoru2& old_dim, const math::vectoru2& new_dim)
	{
		HRESULT res = mpdxgi_swapchain4->ResizeBuffers(
			get_num_backbuffers(),
			new_dim.x,
			new_dim.y,
			translate(get_format()),
			0u // flags
			);

		influx_assert(res == S_OK);
	}

	void dx12_swapchain::destroy_resources(device* device)
	{
		for (auto& resource : get_resources())
		{
			resource->release(device);
		}
	}
}