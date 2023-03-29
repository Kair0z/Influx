#include "InfluxGraphics.h"

#include "Core/Singleton/Singleton.h"

#if INFLUX_GRAPHICS_INCLUDE_DX12
#include "InfluxGraphics/D3D12/D3D12.h"
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN
#include "InfluxGraphics/Vulkan/Vulkan.h"
#endif

namespace Influx::Graphics
{
    // [Global State]
    class GlobalState final 
        : public Singleton<GlobalState>
    {
    public:
        EGraphicsAPI GetCurrentInitializedAPI() const
        {
            return m_currentInitializedAPI;
        }
        bool HasInitializedGraphicsAPI() const
        {
            return GetCurrentInitializedAPI() != EGraphicsAPI::Max;
        }

        bool IsDebugLayerActive() const
        {
            return m_isDebugLayerActive;
        }
        void SetDebugLayerActive(bool isActive)
        {
            m_isDebugLayerActive = isActive;
        }

    private:
        EGraphicsAPI m_currentInitializedAPI = EGraphicsAPI::NotSupported;
        bool m_isDebugLayerActive = false;

        Vector<RHIGraphicsCommandQueueHandle> m_graphicsCommandQueues;

#if INFLUX_GRAPHICS_INCLUDE_DX12
    private:
        Vector<ID3D12Device*> DxLogicalDevices;
        Vector<IDXGIAdapter*> DxgiPhysicalDevices;
        IDXGIFactory2* DxgiFactory2;

        uint32 MainAdapterIndex;

        uint64 DxCachedRtvDescriptorSize = 0;
        uint64 DxCachedDsvDescriptorSize = 0;
        uint64 DxCachedResourceDescriptorSize = 0;
        uint64 DxCachedSamplerDescriptorSize = 0;

        constexpr static uint8 k_dxMaxNumSamplerDescriptorsPerHeap = 16u;
        constexpr static uint8 k_dxMaxNumResourceDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumRtvDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumDsvDescriptorsPerHeap = 64u;

    public:
        EResult InitializeDx12()
        {
            m_currentInitializedAPI = EGraphicsAPI::D3D12;

            DxgiFactory2 = D3D12::Factory::CreateTier2(INFLUX_GRAPHICS_DEBUG);
            DxgiPhysicalDevices = D3D12::Adapter::SelectAll(DxgiFactory2);
            MainAdapterIndex = 0u; // Temp...

            for (uint64 i = 0u; i < DxgiPhysicalDevices.size(); ++i)
            {
                if (i == MainAdapterIndex)
                {
                    DxLogicalDevices.push_back(D3D12::Device::Create(DxgiPhysicalDevices[i], INFLUX_GRAPHICS_DEBUG));
                }
                else
                {
                    DxLogicalDevices.push_back(nullptr);
                }
            }

            // Cache DescriptorSizes
            DxCachedDsvDescriptorSize       = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            DxCachedResourceDescriptorSize  = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            DxCachedSamplerDescriptorSize   = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            DxCachedRtvDescriptorSize       = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            return EResult(true);
        }

        EResult CleanupDx12()
        {
            for (uint64 i = 0u; i < DxgiPhysicalDevices.size(); ++i)
            {
                if (DxLogicalDevices[i] != nullptr)
                {
                    DxLogicalDevices[i]->Release();
                    DxLogicalDevices[i] = nullptr;
                }
                if (DxgiPhysicalDevices[i] != nullptr)
                {
                    DxgiPhysicalDevices[i]->Release();
                    DxgiPhysicalDevices[i] = nullptr;
                }
            }

            DxgiFactory2->Release();
            return EResult(true);
        }

        ID3D12Device* GetDevice()
        {
            return DxLogicalDevices[MainAdapterIndex];
        }
#endif
    };


    EResult Initialize(EGraphicsAPI api)
    {
        if (GlobalState::Get().HasInitializedGraphicsAPI())
        {
            Cleanup();
        }

        switch (api)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            return GlobalState::Get().InitializeDx12();
#endif
        }

        return EResult(false);
    }

    EResult Cleanup()
    {
        if (!GlobalState::Get().HasInitializedGraphicsAPI())
        {
            return EResult(false);
        }

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            return GlobalState::Get().CleanupDx12();
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            return CleanupVulkan();
#endif
        }

        return EResult();
    }

    EResult Create(EGraphicsAPI api, Function<void()> internalFunc)
    {
        EResult result{};

        if (!(result = Initialize(api)))
        {
            return result;
        }

        internalFunc();

        if (!(result = Cleanup()))
        {
            return result;
        }

        return result;
    }

    EGraphicsAPI GetInitializedGraphicsAPI()
    {
        return GlobalState::Get().GetCurrentInitializedAPI();
    }

    EResult SetDebugLayerEnabled()
    {
        GlobalState::Get().SetDebugLayerActive(true);

        return EResult{};
    }

    bool IsDebugLayerEnabled()
    {
        return GlobalState::Get().IsDebugLayerActive();
    }

    EResult CreateGraphicsCommandQueue(RHIGraphicsCommandQueueHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = D3D12::CreateDxCommandQueue(GlobalState::Get().GetDevice(), 
                D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

            return EResult(true);
#endif
        }

        return EResult(false);
    }

    EResult CreateGraphicsCommandBuffer(RHIGraphicsCommandBufferHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = D3D12::CreateDxCommandAllocator(GlobalState::Get().GetDevice(),
                D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

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

            out_handle = D3D12::CreateDxCommandList(GlobalState::Get().GetDevice(),
                D3D12::CreateDxCommandAllocator(GlobalState::Get().GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT), D3D12_COMMAND_LIST_TYPE_DIRECT);

            return EResult(true);
#endif
        }

        return EResult(false);
    }

    EResult CreateSwapchain(const RHISwapchainDesc& desc, RHISwapchainHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            uint8 numBuffers = 0u;
            switch (desc.Buffering)
            {
            case RHISwapchainDesc::EBuffering::Single: numBuffers = 1u; break;
            case RHISwapchainDesc::EBuffering::Double: numBuffers = 2u; break;
            case RHISwapchainDesc::EBuffering::Triple: numBuffers = 3u; break;
            }

            // out_handle = D3D12::Swapchain::CreateTier3(s_GlobalState.DxgiFactory2, (::HWND)desc, dxCommandQueue->GetDxCommandQueue(),
            //     desc.Dimensions.x, desc.Dimensions.y, numBuffers, Conversion::ToDx12(result->m_renderTargetFormat));

            INFLUX_GRAPHICS_TODO;
            return EResult(false);
#endif
        }

        return EResult(false);
    }

    EResult DispatchSwapchainPresent()
    {
        return EResult();
    }

    EResult DispatchGraphicsCommands(Function<void()> commands)
    {
        if (RHIGraphicsCommandQueueHandle commandQueue; CreateGraphicsCommandQueue(commandQueue))
        {
            if (RHIGraphicsCommandListHandle commandList; CreateGraphicsCommandList(commandList))
            {

                commands();
            }
        }
        
        return EResult();
    }

    EResult GraphicsCmd_ClearRenderTargetView()
    {
        return EResult();
    }
}