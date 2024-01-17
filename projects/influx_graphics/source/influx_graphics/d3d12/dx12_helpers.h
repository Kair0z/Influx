#pragma once

#include "dx12_headers.h"
#include "dx12_conversion.h"

// core/vector
#include "core/container/vector.h"

namespace influx::graphics::dx12helpers
{
    bool adapter_is_software(IDXGIAdapter1* adapter);

    bool adapter_supports_dx12(IDXGIAdapter1* adapter);

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    void set_debug_layer_enabled(bool enabled);

    ID3D12CommandQueue* create_command_queue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, int priority = 1);

    ID3D12CommandAllocator* create_command_allocator(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

    ID3D12DescriptorHeap* create_descriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 capacity);

    D3D12_CPU_DESCRIPTOR_HANDLE create_rtv(ID3D12Device* device, ID3D12Resource* resource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, DXGI_FORMAT format);

    struct descriptor_strides final
    {
        size_t m_rtv{};
        size_t m_dsv{};
        size_t m_cbv{};
        size_t m_sampler{};
    };

    descriptor_strides query_descriptor_strides(ID3D12Device* device);

    // Helper function for acquiring the all available hardware adapters that supports Direct3D 12. (sorted on performance)
    template <typename _adapter_t>
    inline vector<_adapter_t*> get_hardware_adapters(IDXGIFactory1* pFactory)
    {
        vector<_adapter_t*> result_adapters{};
        const bool prefer_performance = true;

        IDXGIAdapter1* adapter = nullptr;
        IDXGIFactory6* factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    prefer_performance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                    ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (adapter_is_software(adapter))
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                //if (adapter_supports_dx12(adapter))
                {
                    result_adapters.push_back(adapter);
                }
            }
        }

        return result_adapters;
    }

    template <typename _device_t>
    inline _device_t* create_logical_device(IDXGIAdapter1* adapter)
    {
        _device_t* result_device = nullptr;

        // this better succeed...
        ::D3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&result_device));

        return result_device;
    }

    template <typename _swapchain_t>
    inline _swapchain_t* create_swapchain(IDXGIFactory2* factory, ID3D12CommandQueue* queue, ::HWND window,
        uint32_t width, uint32_t height, DXGI_FORMAT format, uint32 num_buffers)
    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.BufferCount = num_buffers;
        desc.Width = width;
        desc.Height = height;
        desc.Format = format;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.SampleDesc.Count = 1;

        _swapchain_t* result_swapchain = nullptr;
        IDXGISwapChain1* int_swapchain;
        factory->CreateSwapChainForHwnd(
            queue,
            window,
            &desc,
            nullptr,
            nullptr,
            &int_swapchain);

        // does not support fullscreen transitions.
        factory->MakeWindowAssociation((::HWND)window, DXGI_MWA_NO_ALT_ENTER);
        result_swapchain = (_swapchain_t*)int_swapchain;

        return result_swapchain;
    }

    template <typename _cmdlist_t>
    inline _cmdlist_t* create_command_list(ID3D12Device* device, ID3D12CommandAllocator* allocator,
        D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* init_pipeline_state = nullptr)
    {
        _cmdlist_t* result_cmdlist = nullptr;
        device->CreateCommandList(0u, type, allocator, init_pipeline_state, IID_PPV_ARGS(&result_cmdlist));
        return result_cmdlist;
    }

    template <typename _fence_t>
    inline _fence_t* create_fence(ID3D12Device* device)
    {
        _fence_t* result_fence = nullptr;
        D3D12_FENCE_FLAGS flags{};
        device->CreateFence(0u, flags, IID_PPV_ARGS(&result_fence));
        return result_fence;
    }

    template <typename _resource_t>
    inline _resource_t* create_tex2d_resource(ID3D12Device* device, DXGI_FORMAT format, uint64_t width, uint64_t height,
        uint16_t array_size, uint16_t mip_levels, uint32_t sample_count, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES init_state)
    {
        _resource_t* result_resource = nullptr;

        // heap desc
        auto heap_properties = D3D12_HEAP_PROPERTIES{};

        // resource desc
        auto layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        auto alignment = 0u;
        auto resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, static_cast<uint32>(height),
            array_size, mip_levels, sample_count, 0u, flags, layout, alignment);

        device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
            &resource_desc, init_state, nullptr, IID_PPV_ARGS(&result_resource));

        return result_resource;
    }
}