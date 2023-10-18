#include "renderer_pch.h"
#include "renderer.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D3DCompiler.lib")
#include "foreign/d3dx12.h"

#include <thread>

namespace influx::renderer
{
    // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
    // If no such adapter can be found, *ppAdapter will be set to nullptr.
    inline void get_hardware_adapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        IDXGIAdapter1* adapter = nullptr;

        IDXGIFactory6* factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter;
    }

	void renderer_state::initialize(const init_args& args)
	{
        UINT dxgiFactoryFlags = 0;

        // debug layer
#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ID3D12Debug* debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif
        // factory
        IDXGIFactory4* factory;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
        mpdx_factory = factory;

        // device
        if (k_useWarp)
        {
            IDXGIAdapter* warpAdapter;
            factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

            D3D12CreateDevice(
                warpAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&mpdx_device));
        }
        else
        {
            IDXGIAdapter1* hardwareAdapter;
            get_hardware_adapter(factory, &hardwareAdapter, true);

            D3D12CreateDevice(
                hardwareAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&mpdx_device));
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        mpdx_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpdx_commandQueue));

		m_is_initialized = true;
	}

	void renderer_state::present_to_window(platform::window_handle window_handle, const present_args& args)
	{
        // recreate if necessary
        recreate_swapchain_from_window(e_buffering::tripple, window_handle);

        if (mpdx_swapchain != nullptr)
        {
            mpdx_swapchain->Present(args.m_vsync ? 1u : 0u, 0u);
        }
	}

	bool renderer_state::is_initialized() const
	{
		return m_is_initialized;
	}

	void renderer_state::cleanup()
	{
		m_is_initialized = false;
	}

    command_list* renderer_state::record()
    {
        return nullptr;
    }

    void renderer_state::submit(const command_list* list)
    {
        submit({ list });
    }

    void renderer_state::submit(const vector<command_list*> lists)
    {
        if (mpdx_commandQueue == nullptr)
        {
            return;
        }

        mpdx_commandQueue->ExecuteCommandLists(1u, nullptr);
    }

    void renderer_state::recreate_swapchain_from_window(const e_buffering& buffering, platform::window_handle handle)
    {
        math::rectu window_rect = platform::get_windowrect_client<uint32>(handle);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = ::UINT(buffering);
        swapChainDesc.Width = ::UINT(window_rect.m_width_height.x);
        swapChainDesc.Height = ::UINT(window_rect.m_width_height.y);
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        IDXGISwapChain1* swapChain;
        mpdx_factory->CreateSwapChainForHwnd(
            mpdx_commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
            (::HWND)handle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain);

        // does not support fullscreen transitions.
        mpdx_factory->MakeWindowAssociation((::HWND)handle, DXGI_MWA_NO_ALT_ENTER);

        mpdx_swapchain = (IDXGISwapChain4*)swapChain;
        m_swapchain_buffer_idx = mpdx_swapchain->GetCurrentBackBufferIndex();
    }


#pragma region frontend_api
    void initialize(const init_args& args)
    {
        renderer_state::get_instance().initialize(args);
    }

    command_list* record()
    {
        return renderer_state::get_instance().record();
    }

    void submit(const command_list* list)
    {
        renderer_state::get_instance().submit(list);
    }

    void submit(const vector<command_list*>& lists)
    {
        renderer_state::get_instance().submit(lists);
    }

    void present_to_window(platform::window_handle window_handle, const present_args& args)
    {
        renderer_state::get_instance().present_to_window(window_handle, args);
    }

    bool is_initialized()
    {
        return renderer_state::get_instance().is_initialized();
    }

    void cleanup()
    {
        renderer_state::get_instance().cleanup();
    }
#pragma endregion

#if 0
	Result Render()
	{
		Result result{};

		Graphics::Result gfxResult{};

		// Wait until the Graphics queue finished LAST frame
		const uint64 previousFrame = (GlobalState::GetFrameIndexReference() != 0u) ? GlobalState::GetFrameIndexReference() - 1u : 0u;
		gfxResult = Graphics::WaitForGraphicsQueueSignal(previousFrame);

		gfxResult = Graphics::DispatchGraphicsCommands([](const Graphics::RHICommandListHandle& cmdList)
		{
			if (IsAttachedToWindow(nullptr))
			{
				const Vectorf4 clearColour = { 1,0,0,1 };

				Graphics::Commands::ClearSwapchainBackBuffer(cmdList, GlobalState::GetAttachedSwapchain(), clearColour);
			}
		}
		, previousFrame + 1u);

		// Increase frame value...
		++GlobalState::GetFrameIndexReference();

		return result;
	}

	Result Present()
	{
		Result result{};

		if (!IsAttachedToWindow(nullptr))
		{
			return Result{};
		}

		constexpr static bool Vsync = true;
		Graphics::DispatchSwapchainPresent(GlobalState::GetAttachedSwapchain(), { Vsync });

		return result;
	}

	Result Initialize()
	{
		Result result{};

		Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);
		GlobalState::SetInitialized(true);

		return result;
	}

	bool IsInitialized()
	{
		return GlobalState::IsInitialized();
	}

	Result Cleanup()
	{
		Result result{};

		Graphics::Cleanup();
		GlobalState::SetInitialized(false);
		
		return result;
	}

	Result AttachToWindow(platform::window_handle window)
	{
		Result result{};

		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Graphics::RHISwapchainDesc::EBuffering::Triple;
		swapchainDesc.Dimensions	= platform::GetClientWindowDimensions<uint32>(window);
		swapchainDesc.WindowHandle	= window;

		Graphics::Result gfxResult{};
		Graphics::RHISwapchainHandle out_handle;
		gfxResult = Graphics::CreateSwapchain(swapchainDesc, out_handle);

		GlobalState::SetAttachedSwapchain(out_handle);

		return result;
	}

	bool IsAttachedToWindow(platform::window_handle window)
	{
		return GlobalState::HasAttachedSwapchain();
	}
#endif
}

