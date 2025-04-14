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
		mp_resources = create_resources(swapchain_dependencies.mp_device).get();
	}

	dx12_swapchain::~dx12_swapchain()
	{
	}

	result<> dx12_swapchain::present(const present_args& args)
	{
		HRESULT res = mpdxgi_swapchain4->Present(args.m_vsync ? 1 : 0, 0u);
		if (res != S_OK)
		{
			return result<>::make_error("error: IDXGISwapchain->Present() failed!");
		}
		return {};
	}

	result<> dx12_swapchain::acquire_backbuffer()
	{
		return {};
	}

	result<uint8> dx12_swapchain::get_current_backbuffer_index()
	{
		m_current_backbuffer_index = mpdxgi_swapchain4->GetCurrentBackBufferIndex();
		return m_current_backbuffer_index;
	}

	void dx12_swapchain::release_impl(device*)
	{
		mpdxgi_swapchain4->Release();
	}

	result<vector<resource*>> dx12_swapchain::create_resources(device* device)
	{
		using result_type = result<vector<resource*>>;
		const auto& dimensions = get_dimensions();
	
		HRESULT hres = {};

		vector<resource*> resources{};
		for (uint8 i = 0u; i < get_num_backbuffers(); ++i)
		{
			ID3D12Resource* dxresource = nullptr;
			hres = mpdxgi_swapchain4->GetBuffer(i, IID_PPV_ARGS(&dxresource));
			if (hres != S_OK)
			{
				return result_type::make_error("error: IDXGISwapchain->GetBuffer() failed!");
			}

			tex2D_desc desc{};
			desc.m_format = get_format();
			desc.m_dimensions = dimensions;
			desc.m_init_state = e_resource_state::present;

			// import the acquired resource to our device for bookkeeping
			resource* new_resource = device->import_texture(dxresource, desc);
			resources.push_back(new_resource);
		}
		
		mp_resources = resources;
		return resources;
	}

	result<> dx12_swapchain::resize_impl(const math::vectoru2& old_dim, const math::vectoru2& new_dim)
	{
		HRESULT hres = mpdxgi_swapchain4->ResizeBuffers(
			get_num_backbuffers(),
			new_dim.x,
			new_dim.y,
			translate(get_format()),
			0u // flags
			);

		if (hres != S_OK)
		{
			return result<>::make_error("error: IDXGISwapchain::ResizeBuffers() failed!");
		}

		return {};
	}

	result<> dx12_swapchain::destroy_resources(device* device)
	{
		for (auto& resource : get_resources())
		{
			resource->release(device);
		}

		return {};
	}
}