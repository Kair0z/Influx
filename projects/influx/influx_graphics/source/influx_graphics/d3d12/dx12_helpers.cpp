#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_helpers.h"

namespace influx::graphics::dx12helpers
{
    bool adapter_is_software(IDXGIAdapter1* adapter)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        return desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
    }

    bool adapter_supports_dx12(IDXGIAdapter1* adapter)
    {
        return SUCCEEDED(::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr));
    }

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    void set_debug_layer_enabled(bool enabled)
    {
        UINT dxgiFactoryFlags = 0;
        
        ID3D12Debug* debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            if (enabled)
            {
                debugController->EnableDebugLayer();
            }
            else
            {
                debugController->Release();
            }
            

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }

        
    }

    ID3D12CommandQueue* create_command_queue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, int priority)
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Priority = priority;
        desc.Type = type;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        ID3D12CommandQueue* result_queue = nullptr;
        device->CreateCommandQueue(&desc, IID_PPV_ARGS(&result_queue));
        return result_queue;
    }

    ID3D12CommandAllocator* create_command_allocator(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        ID3D12CommandAllocator* allocator = nullptr;
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
        return allocator;
    }

    ID3D12DescriptorHeap* create_descriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 capacity)
    {
        ID3D12DescriptorHeap* result_heap = nullptr;
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Flags;
        desc.NumDescriptors = capacity;
        desc.Type = type;

        device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result_heap));
        return result_heap;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE create_rtv(ID3D12Device* device, ID3D12Resource* resource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, DXGI_FORMAT format)
    {
        D3D12_RENDER_TARGET_VIEW_DESC desc{};
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0u;
        desc.Texture2D.PlaneSlice = 0u;
        desc.Format = format;

        device->CreateRenderTargetView(resource, &desc, cpu_handle);
        return cpu_handle;
    }

    descriptor_strides query_descriptor_strides(ID3D12Device* device)
    {
        descriptor_strides strides{};
        strides.m_rtv = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        strides.m_dsv = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        strides.m_cbv = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        strides.m_sampler = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        return strides;
    }
}