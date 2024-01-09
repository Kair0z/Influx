#include "renderer_pch.h"
#include "api.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#include "d3dx12.h"

namespace influx::renderer::api
{
#pragma region misc
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

#pragma endregion

#pragma region base object
	template <typename _t>
	_t* _get_native(const base_object& obj)
	{
		return reinterpret_cast<_t*>(obj.get_native());
	}

	template <typename _t>
	_t*& _get_native(base_object& obj)
	{
		return reinterpret_cast<_t*&>(obj.get_native());
	}
#pragma endregion

#pragma region logic device
	logical_device::logical_device()
		: base_object()
	{
        UINT dxgiFactoryFlags = 0;
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

        IDXGIFactory4* factory = nullptr;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
        IDXGIAdapter1* hardwareAdapter;
        get_hardware_adapter(factory, &hardwareAdapter, true);

		D3D12CreateDevice(
			hardwareAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&_get_native<ID3D12Device>(*this)));
	}
#pragma endregion

#pragma region command queue
	D3D12_COMMAND_QUEUE_DESC build(const command_queue_desc& desc)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		return queueDesc;
	}

	command_queue::command_queue(const logical_device& device, const command_queue_desc& desc)
		: base_object()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = build(desc);
		_get_native<ID3D12Device>(device)->CreateCommandQueue(&queueDesc, 
			IID_PPV_ARGS(&_get_native<ID3D12CommandQueue>(*this)));
	}
#pragma endregion
}