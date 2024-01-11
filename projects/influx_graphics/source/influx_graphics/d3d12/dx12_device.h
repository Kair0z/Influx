#include "influx_graphics/device.h"

// dx12 includes
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

// helpers
#include "influx_graphics/d3d12/dx12_helpers.h"

// subheaders
#include "influx_graphics/d3d12/dx12_commandqueue.h"
#include "influx_graphics/d3d12/dx12_fence.h"
#include "influx_graphics/d3d12/dx12_swapchain.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "influx_graphics/d3d12/dx12_allocator.h"

namespace influx::graphics
{
	class dx12_device final
		: public device
	{
	public:
		dx12_device()
			: device()
		{
			// create factory
			CreateDXGIFactory2(0u, IID_PPV_ARGS(&mpdxgi_factory));

			// query adapters
			auto adapters = dx12helpers::get_hardware_adapters<IDXGIAdapter1>(mpdxgi_factory);
			for (size_t i = 0u; i < adapters.size(); ++i)
			{
				mpdxgi_adapters.push_back(adapters[i]);
			}

			// create dx12 logical device of first adapter
			ID3D12Device* dxdevice = 
				dx12helpers::create_logical_device<ID3D12Device>(mpdxgi_adapters[0u]);
		}

		// get info about physical devices:
		virtual vector<physical_device_info> get_gpu_infos()
		{
			vector<physical_device_info> result_infos{};
			
			for (size_t i = 0u; i < mpdxgi_adapters.size(); ++i)
			{
				
			}

			return result_infos;
		}

		// get interface to graphics object creation:
		virtual command_queue* create_command_queue(const command_queue_desc& desc) override
		{
			auto dxcommandqueue = dx12helpers::create_command_queue(
				mpdx_devices[0u], convert(desc.m_type), static_cast<int>(desc.m_priority));

			return new dx12_commandqueue(desc, dxcommandqueue);
		}

		virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window) override
		{
			// parse window
			swapchain_desc desc{};
			auto rect = platform::get_windowrect_client<uint32>(window);
			uint32 width = rect.m_width_height.x;
			uint32 height = rect.m_width_height.y;
			e_format format = e_format::rgba8;

			// create dx swapchain
			IDXGISwapChain4* dxswapchain = dx12helpers::create_swapchain<IDXGISwapChain4>(
				mpdxgi_factory, queue->get_native<ID3D12CommandQueue>(), 
				(::HWND)window, width, height, convert(format), 2u);

			return new dx12_swapchain(desc, dxswapchain);
		}

		virtual command_allocator* create_graphics_allocator() override
		{
			auto dxallocator = dx12helpers::create_command_allocator(mpdx_devices[0u], D3D12_COMMAND_LIST_TYPE_DIRECT);
			return new dx12_command_allocator(dxallocator);
		}

		virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state = nullptr) override
		{
			auto dxcommandlist = dx12helpers::create_command_list<ID3D12GraphicsCommandList>(mpdx_devices[0u],
				allocator->get_native<ID3D12CommandAllocator>(), D3D12_COMMAND_LIST_TYPE_DIRECT,
				init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);

			return new dx12_commandlist(dxcommandlist);
		}

		virtual fence* create_fence() override
		{
			return new dx12_fence(dx12helpers::create_fence<ID3D12Fence>(mpdx_devices[0u]));
		}

		virtual resource* create_resource(const tex2D_desc& desc) override
		{
			auto dxresource = dx12helpers::create_tex2d_resource<ID3D12Resource>( mpdx_devices[0u],
				convert(desc.m_format), desc.m_dimensions.x, desc.m_dimensions.y, desc.m_arraysize, desc.m_num_mips,
				desc.m_sample_count, convert(desc.m_flags), D3D12_RESOURCE_STATE_COMMON);

			return new dx12_resource(dxresource, desc);
		}

	private:
		IDXGIFactory2* mpdxgi_factory;
		vector<IDXGIAdapter1*> mpdxgi_adapters;
		vector<ID3D12Device*> mpdx_devices;
	};
}