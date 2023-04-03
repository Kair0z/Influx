#include "InfluxGraphics.h"

#include "Core/Singleton/Singleton.h"

#if INFLUX_GRAPHICS_INCLUDE_DX12
#include "InfluxGraphics/D3D12/D3D12.h"
#include "InfluxGraphics/D3D12/D3D12Conversion.h"
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
        using ObjectContainer = Vector<IRHIObjectHandle>;

    public:
        static EGraphicsAPI GetCurrentInitializedAPI()
        {
            return Get().m_currentInitializedAPI;
        }

        static bool HasInitializedGraphicsAPI()
        {
            return Get().GetCurrentInitializedAPI() != EGraphicsAPI::Max;
        }

        static bool IsDebugLayerActive()
        {
            return Get().m_isDebugLayerActive;
        }

        static void SetDebugLayerActive(bool isActive)
        {
            Get().m_isDebugLayerActive = isActive;
        }
        
        template <ERHIObject _E>
        static bool CanRegisterRHIObject()
        {
            constexpr uint8 idx = static_cast<uint8>(_E);
            return Get().m_objectLists[idx].size() < k_maxNumRHIObjectsPerType[idx];
        }

        template <class _E>
        static bool CanRegisterRHIObject()
        {
            return CanRegisterRHIObject<_E::GetStaticType()>();
        }

        /* Register a raw-object pointer as an RHI object */
        template <class _E>
        static _E RegisterRHIObject(void* rawObjectPointer)
        {
            static_assert(std::is_base_of_v<IRHIObjectHandle, _E> == true, "RegisterRHIObject [_E] must derive from IRHIObjectHandle!");

            if (!CanRegisterRHIObject<_E>())
            {
                return nullptr;
            }

            constexpr uint8 idx = static_cast<uint8>(_E::GetStaticType());

            // the constructor of _E takes in a void*...
            Get().m_objectLists[idx].push_back(_E(rawObjectPointer));
            return _E(rawObjectPointer);
        }

        /* Pass a raw-object-creating function that after creation will Register it as an RHI object*/
        template <class _E>
        static _E CreateAndRegisterRHIObject(Function<void*()> creationCallback)
        {
            static_assert(std::is_base_of_v<IRHIObjectHandle, _E> == true, "CreateAndRegisterRHIObject [_E] must derive from IRHIObjectHandle!");

            if (!CanRegisterRHIObject<_E>())
            {
                return _E{ nullptr };
            }

            return RegisterRHIObject<_E>(creationCallback());
        }

        /* Get all RHI Objects registered based on the ERHIObject enum */
        template <ERHIObject _E>
        static const ObjectContainer& GetRHIObjectsOfType()
        {
            return Get().m_objectLists[static_cast<uint8>(_E)];
        }

        /* Get all RHI Objects based on their type */
        template <class _E>
        static const ObjectContainer& GetRHIObjectsOfType()
        {
            static_assert(std::is_base_of_v<IRHIObjectHandle, _E> == true, "GetRHIObjectsOfType [_E] must derive from IRHIObjectHandle!");

            return GetRHIObjectsOfType<_E::GetStaticType()>();
        }

        template <ERHIObject _E>
        static bool HasObjectOfType()
        {
            return GetRHIObjectsOfType<_E>().size() != 0u;
        }

        /* GetRHIObjectsOfType[idx]*/
        template <class _E>
        static _E GetRHIObjectOfType(uint8 idx)
        {
            static_assert(std::is_base_of_v<IRHIObjectHandle, _E> == true, "GetRHIObjectOfType [_E] must derive from IRHIObjectHandle!");

            if (idx < GetRHIObjectsOfType<_E>().size())
            {
                const IRHIObjectHandle& handle = Get().m_objectLists[static_cast<uint8>(_E::GetStaticType())][idx];
                return _E{ handle.GetInternal() };
            }
            
            return _E{ nullptr };
        }

    private:
        EGraphicsAPI m_currentInitializedAPI = EGraphicsAPI::NotSupported;
        bool m_isDebugLayerActive = false;

        Array<ObjectContainer, static_cast<uint8>(ERHIObject::Max)> m_objectLists;

#if INFLUX_GRAPHICS_INCLUDE_DX12
    private:
        Vector<ID3D12Device*> DxLogicalDevices;
        Vector<IDXGIAdapter*> DxgiPhysicalDevices;
        IDXGIFactory2* DxgiFactory2;

        uint32 MainAdapterIndex;

    public:
        EResult InitializeDx12()
        {
            m_currentInitializedAPI = EGraphicsAPI::D3D12;

            DxgiFactory2 = D3D12::Factory::CreateTier2(m_isDebugLayerActive);
            DxgiPhysicalDevices = D3D12::Adapter::SelectAll(DxgiFactory2);
            MainAdapterIndex = 0u; // Temp...

            for (uint64 i = 0u; i < DxgiPhysicalDevices.size(); ++i)
            {
                if (i == MainAdapterIndex)
                {
                    DxLogicalDevices.push_back(D3D12::Device::Create(DxgiPhysicalDevices[i], m_isDebugLayerActive));
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

        static ID3D12Device* GetDevice()
        {
            return Get().DxLogicalDevices[Get().MainAdapterIndex];
        }

        static IDXGIFactory2* GetFactory2()
        {
            return Get().DxgiFactory2;
        }

        uint64 DxCachedRtvDescriptorSize = 0;
        uint64 DxCachedDsvDescriptorSize = 0;
        uint64 DxCachedResourceDescriptorSize = 0;
        uint64 DxCachedSamplerDescriptorSize = 0;

        constexpr static uint8 k_dxMaxNumSamplerDescriptorsPerHeap = 16u;
        constexpr static uint8 k_dxMaxNumResourceDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumRtvDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumDsvDescriptorsPerHeap = 64u;
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN

#endif
    };


    EResult Initialize(EGraphicsAPI api)
    {
        EResult result{ false };

        if (GlobalState::HasInitializedGraphicsAPI())
        {
            result = Cleanup();
        }

        switch (api)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            result = GlobalState::Get().InitializeDx12();
#endif
        }

        return result;
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
        return GlobalState::GetCurrentInitializedAPI();
    }

    EResult SetDebugLayerEnabled()
    {
        GlobalState::SetDebugLayerActive(true);

#if INFLUX_GRAPHICS_INCLUDE_DX12
        D3D12::EnableDxDebugLayer();
#endif

        return EResult{true};
    }

    bool IsDebugLayerEnabled()
    {
        return GlobalState::IsDebugLayerActive();
    }

    EResult CreateGraphicsCommandQueue(RHIGraphicsCommandQueueHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIGraphicsCommandQueueHandle>([]()
                {
                    return D3D12::CreateDxCommandQueue(GlobalState::GetDevice(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
                });

            break;
#endif
        }

        return EResult(true);
    }

    EResult WaitForGraphicsCommandQueueToFinish(const RHIGraphicsCommandQueueHandle& commandQueueHandle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            INFLUX_GRAPHICS_TODO;
            break;
#endif
        }
        
        return EResult(true);
    }

    EResult CreateGraphicsCommandBuffer(RHIGraphicsCommandBufferHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIGraphicsCommandBufferHandle>([]()
                {
                    return D3D12::CreateDxCommandAllocator(GlobalState::GetDevice(),
                        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
                });

            break;
#endif
        }

        return EResult(true);
    }

    EResult CreateGraphicsCommandList(RHIGraphicsCommandBufferHandle& out_existingCommandBuffer, RHIGraphicsCommandListHandle& out_handle)
    {
        // If an invalid / null command-buffer is passed, create a new one...
        if (!out_existingCommandBuffer.IsValid())
            CreateGraphicsCommandBuffer(out_existingCommandBuffer);

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIGraphicsCommandListHandle>([&out_existingCommandBuffer]()
                {
                    return D3D12::CreateDxCommandList(GlobalState::GetDevice(), 
                        out_existingCommandBuffer.GetInternal<ID3D12CommandAllocator>(),
                        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
                });

            break;
#endif
        }

        // Reset Graphics Command List with the associated Command Buffer...
        ResetGraphicsCommandlist(out_handle, out_existingCommandBuffer);

        return EResult(true);
    }

    EResult ResetGraphicsCommandlist(const RHIGraphicsCommandListHandle& commandListHandle, const RHIGraphicsCommandBufferHandle& commandbufferHandle)
    {
        INFLUX_GRAPHICS_ASSERT(commandListHandle);
        INFLUX_GRAPHICS_ASSERT(commandbufferHandle);

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            GetInternalType<ID3D12GraphicsCommandList>(commandListHandle)->Close();
            GetInternalType<ID3D12GraphicsCommandList>(commandListHandle)->Reset(commandbufferHandle.GetInternal<ID3D12CommandAllocator>(), nullptr);

            break;
#endif
        }

        INFLUX_GRAPHICS_ASSERT(false);
        return { false };
    }

    EResult DispatchGraphicsCommandListToGpu(const RHIGraphicsCommandListHandle& commandListHandle, const RHIGraphicsCommandQueueHandle& commandQueueHandle)
    {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        if (ID3D12GraphicsCommandList* d3d12GfxCmdList = commandListHandle.GetInternal<ID3D12GraphicsCommandList>())
        {
            if (ID3D12CommandQueue* d3d12CommandQueue = commandQueueHandle.GetInternal<ID3D12CommandQueue>())
            {
                d3d12GfxCmdList->Close();

                ID3D12CommandList* d3d12CmdLists[1u]{ d3d12GfxCmdList };

                d3d12CommandQueue->ExecuteCommandLists(1u, d3d12CmdLists);
                return {};
            }
        }
#endif

        INFLUX_GRAPHICS_TODO;
        return { false };
    }

    EResult CreateSwapchain(const RHISwapchainDesc& desc, RHISwapchainHandle& out_handle)
    {
        uint8 numBuffers = 0u;
        switch (desc.Buffering)
        {
        case RHISwapchainDesc::EBuffering::Single: numBuffers = 1u; break;
        case RHISwapchainDesc::EBuffering::Double: numBuffers = 2u; break;
        case RHISwapchainDesc::EBuffering::Triple: numBuffers = 3u; break;
        }

        // Get an existing, or create a new graphics command queue...
        RHIGraphicsCommandQueueHandle cmdQueueHandle = GlobalState::GetRHIObjectOfType<RHIGraphicsCommandQueueHandle>(0u);
        if (!cmdQueueHandle.IsValid())
        {
            CreateGraphicsCommandQueue(cmdQueueHandle);
        }

        INFLUX_GRAPHICS_ASSERT(cmdQueueHandle.IsValid());

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHISwapchainHandle>([&desc, &cmdQueueHandle, numBuffers]()
                {
                    return D3D12::Swapchain::CreateTier3(GlobalState::GetFactory2(), (::HWND)desc.WindowHandle, cmdQueueHandle.GetInternal<ID3D12CommandQueue>(),
                        desc.Dimensions.x, desc.Dimensions.y, numBuffers);
                });

            INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());

            // Register buffer-resources to RHI:
            for (uint8 i = 0; i < numBuffers; ++i)
            {
                ID3D12Resource* resource;
                if (out_handle.GetInternal<IDXGISwapChain3>()->GetBuffer(i, IID_PPV_ARGS(&resource)))
                {
                    GlobalState::RegisterRHIObject<RHIBufferHandle>(resource);
                }
                else INFLUX_GRAPHICS_ASSERT(false);


            }

            break;
#endif
        }

        return EResult(true);
    }

    EResult DispatchSwapchainPresent(const RHISwapchainHandle& swapchain)
    {
        return EResult();
    }

    EResult CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIDescriptorHeapHandle>([&desc]()
                {
                    D3D12_DESCRIPTOR_HEAP_TYPE type{};
                    uint8 numDescriptors{};
                    bool isShaderVisible = false;
                    switch (desc.Type)
                    {
                    case ERHIResourceViewType::Resource:
                        type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                        numDescriptors = GlobalState::k_dxMaxNumResourceDescriptorsPerHeap;
                        isShaderVisible = true;
                        break;

                    case ERHIResourceViewType::DSV:
                        type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                        numDescriptors = GlobalState::k_dxMaxNumDsvDescriptorsPerHeap;
                        isShaderVisible = false;
                        break;

                    case ERHIResourceViewType::RTV:
                        type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                        numDescriptors = GlobalState::k_dxMaxNumRtvDescriptorsPerHeap;
                        isShaderVisible = false;
                        break;

                    case ERHIResourceViewType::Sampler:
                        type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                        numDescriptors = GlobalState::k_dxMaxNumSamplerDescriptorsPerHeap;
                        isShaderVisible = true;
                        break;
                    }

                    return D3D12::CreateDxDescriptorHeap(GlobalState::GetDevice(), type, numDescriptors,
                        isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
                });

            break;
#endif
        }

        return EResult(true);
    }

    EResult CreateRenderTargetView(const RHIDescriptorHeapHandle& descriptorHeap)
    {
        return EResult();
    }

    EResult DispatchGraphicsCommands(Function<void(const RHIGraphicsCommandListHandle&)> commands)
    {
        if (commands == nullptr)
        {
            return { false };
        }

        // Get an existing, or create a new graphics command queue...
        RHIGraphicsCommandQueueHandle cmdQueueHandle = GlobalState::GetRHIObjectOfType<RHIGraphicsCommandQueueHandle>(0u);
        if (!cmdQueueHandle.IsValid())
        {
            CreateGraphicsCommandQueue(cmdQueueHandle);
        }

        // List of commands we'll pass on for the entire 'frame'
        RHIGraphicsCommandListHandle cmdListHandle;

        // Memory allocator for the whole 'frame'
        RHIGraphicsCommandBufferHandle cmdBufferHandle;

        if (CreateGraphicsCommandList(cmdBufferHandle, cmdListHandle))
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            // ...
#endif

            // Record commands...
            commands(cmdListHandle);

            DispatchGraphicsCommandListToGpu(cmdListHandle, cmdQueueHandle);
        }
        
        return EResult();
    }

    namespace Commands
    {
        EResult ClearRenderTargetView(const RHIGraphicsCommandListHandle& cmdListHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            if (ID3D12GraphicsCommandList* d3d12CmdList = cmdListHandle.GetInternal<ID3D12GraphicsCommandList>())
            {
                // d3d12CmdList->ClearRenderTargetView()
            }
#endif

            return { true };
        }

        EResult ClearSwapchainBackBuffer(const RHIGraphicsCommandListHandle& cmdListHandle, const RHISwapchainHandle& swapchainHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            if (ID3D12GraphicsCommandList* gfxCmdList = cmdListHandle.GetInternal<ID3D12GraphicsCommandList>())
            {
                if (IDXGISwapChain3* swapchain = swapchainHandle.GetInternal<IDXGISwapChain3>())
                {
                    ID3D12Resource* bufferResource;
                    swapchain->GetBuffer(swapchain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&bufferResource));

                    D3D12_CPU_DESCRIPTOR_HANDLE resultRenderTargetView{};

                    // Create the rendertargetview for buffer
                    GlobalState::GetDevice()->CreateRenderTargetView(bufferResource, nullptr, resultRenderTargetView);

                    gfxCmdList->ClearRenderTargetView(resultRenderTargetView, colour.data, 0u, nullptr);

                    // Do we need to?
                    bufferResource->Release();
                }
            }
#endif

            return { true };
        }
    }
}