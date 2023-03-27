#include "InfluxGraphics.h"

#if INFLUX_GRAPHICS_INCLUDE_DX12
#include "InfluxGraphics/D3D12/D3D12.h"
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN
#include "InfluxGraphics/Vulkan/Vulkan.h"
#endif

namespace Influx::Graphics
{
    struct
    {
        EGraphicsAPI CurrentInitializedAPI;
        bool IsDebugLayerActive;

        bool HasInitializedGraphicsAPI() const
        {
            return CurrentInitializedAPI != EGraphicsAPI::Max;
        }

#if INFLUX_GRAPHICS_INCLUDE_DX12
        Vector<ID3D12Device*> DxLogicalDevices;
        Vector<IDXGIAdapter*> DxgiPhysicalDevices;
        IDXGIFactory2* DxgiFactory2;

        uint32 MainAdapterIndex;

        uint64 DxCachedRtvDescriptorSize = 0;
        uint64 DxCachedDsvDescriptorSize = 0;
        uint64 DxCachedResourceDescriptorSize = 0;
        uint64 DxCachedSamplerDescriptorSize = 0;

        constexpr static uint8 k_dxMaxNumSamplerDescriptorsPerHeap  = 16u;
        constexpr static uint8 k_dxMaxNumResourceDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumRtvDescriptorsPerHeap      = 64u;
        constexpr static uint8 k_dxMaxNumDsvDescriptorsPerHeap      = 64u;

        ID3D12Device* GetDevice()
        {
            return DxLogicalDevices[MainAdapterIndex];
        }
#endif

    } static s_GlobalState;

#if INFLUX_GRAPHICS_INCLUDE_DX12
    EResult InitializeDx12()
    {
        s_GlobalState.DxgiFactory2          = D3D12::Factory::CreateTier2(INFLUX_GRAPHICS_DEBUG);
        s_GlobalState.DxgiPhysicalDevices   = D3D12::Adapter::SelectAll(s_GlobalState.DxgiFactory2);
        s_GlobalState.MainAdapterIndex      = 0u; // Temp...

        for (uint64 i = 0u; i < s_GlobalState.DxgiPhysicalDevices.size(); ++i)
        {
            if (i == s_GlobalState.MainAdapterIndex)
            {
                s_GlobalState.DxLogicalDevices.push_back(D3D12::Device::Create(s_GlobalState.DxgiPhysicalDevices[i], INFLUX_GRAPHICS_DEBUG));
            }
            else
            {
                s_GlobalState.DxLogicalDevices.push_back(nullptr);
            }
        }

        // Cache DescriptorSizes
        s_GlobalState.DxCachedDsvDescriptorSize         = s_GlobalState.DxLogicalDevices[s_GlobalState.MainAdapterIndex]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        s_GlobalState.DxCachedResourceDescriptorSize    = s_GlobalState.DxLogicalDevices[s_GlobalState.MainAdapterIndex]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        s_GlobalState.DxCachedSamplerDescriptorSize     = s_GlobalState.DxLogicalDevices[s_GlobalState.MainAdapterIndex]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        s_GlobalState.DxCachedRtvDescriptorSize         = s_GlobalState.DxLogicalDevices[s_GlobalState.MainAdapterIndex]->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        return EResult(true);
    }

    EResult CleanupDx12()
    {
        for (uint64 i = 0u; i < s_GlobalState.DxgiPhysicalDevices.size(); ++i)
        {
            if (s_GlobalState.DxLogicalDevices[i] != nullptr)
            {
                s_GlobalState.DxLogicalDevices[i]->Release();
                s_GlobalState.DxLogicalDevices[i] = nullptr;
            }

            if (s_GlobalState.DxgiPhysicalDevices[i] != nullptr)
            {
                s_GlobalState.DxgiPhysicalDevices[i]->Release();
                s_GlobalState.DxgiPhysicalDevices[i] = nullptr;
            }
        }

        s_GlobalState.DxgiFactory2->Release();

        return EResult(true);
    }
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN
    EResult InitializeVulkan()
    {
        return EResult(false);
    }

    EResult CleanupVulkan()
    {
        return EResult(false);
    }
#endif

    EResult Initialize(EGraphicsAPI api)
    {
        if (s_GlobalState.HasInitializedGraphicsAPI())
        {
            Cleanup();
        }

        s_GlobalState.CurrentInitializedAPI = api;

        switch (api)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            return InitializeDx12();     
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            return InitializeVulkan();
#endif
        }

        return EResult(false);
    }

    EResult Cleanup()
    {
        if (!s_GlobalState.HasInitializedGraphicsAPI())
        {
            return EResult(false);
        }

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            return CleanupDx12();
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            return CleanupVulkan();
#endif
        }

        return EResult();
    }

    EGraphicsAPI GetInitializedGraphicsAPI()
    {
        return s_GlobalState.CurrentInitializedAPI;
    }

    EResult SetDebugLayerEnabled()
    {
        s_GlobalState.IsDebugLayerActive = true;
    }

    bool IsDebugLayerEnabled()
    {
        return s_GlobalState.IsDebugLayerActive;
    }

    EResult CreateGraphicsCommandQueue(RHIGraphicsCommandQueueHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = D3D12::CreateDxCommandQueue(s_GlobalState.GetDevice(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
            return EResult(true);
#endif
        }

        return EResult(false);
    }

    EResult CreateGraphicsCommandList(RHIGraphicsCommandListHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = D3D12::CreateDxCommandList(s_GlobalState.GetDevice(), 
                D3D12::CreateDxCommandAllocator(s_GlobalState.GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT), D3D12_COMMAND_LIST_TYPE_DIRECT);

            return EResult(true);
#endif
        }

        return EResult(false);
    }
}