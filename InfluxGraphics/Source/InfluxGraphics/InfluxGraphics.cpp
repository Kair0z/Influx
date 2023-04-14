#include "InfluxGraphics.h"

#if INFLUX_GRAPHICS_USE_CORE
#include "Core/Singleton/Singleton.h"
#endif

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
        struct Child final
        {
            IRHIObjectHandle*       pHandle = nullptr;
            IRHIState*              pState = nullptr;

            friend class GlobalState;
        };

        using ChildContainer = Vector<Child>;

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
        
        /* Check if there's any slots left for given ERHIChild */
        template <ERHIChild _E>
        static bool CanRegisterRHIObject()
        {
            constexpr uint8 idx = static_cast<uint8>(_E);
            return Get().m_childLists[idx].size() < k_maxNumRHIObjectsPerType[idx];
        }
        template <class _E>
        static bool CanRegisterRHIObject()
        {
            return CanRegisterRHIObject<_E::GetStaticType()>();
        }

        /* Register a raw-object pointer as an RHI object */
        template <ERHIChild _E>
        static RHIObjectHandle<_E> TryRegisterRHIObject(void* rawObjectPointer)
        {
            if (!CanRegisterRHIObject<_E>())
            {
                INFLUX_GRAPHICS_ASSERT(false);
                return nullptr;
            }

            constexpr uint8 idx = static_cast<uint8>(_E);

            // Create a new child with handle & null-state
            Child newChild{};
            RHIObjectHandle<_E>* newHandle = new RHIObjectHandle<_E>(rawObjectPointer);
            newChild.pHandle    = newHandle;
            newChild.pState     = new RHIState<_E>{}; // Null...

            Get().m_childLists[idx].push_back(newChild);

            return *newHandle;
        }
        template <class _E>
        static _E TryRegisterRHIObject(void* rawObjectPointer)
        {
            return TryRegisterRHIObject<_E::GetStaticType()>(rawObjectPointer);
        }

        /* Pass a raw-object-creator function and register the result */
        template <ERHIChild _E>
        static RHIObjectHandle<_E> CreateAndRegisterRHIObject(Function<void*()> creator)
        {
            if (!CanRegisterRHIObject<_E>())
            {
                // ! passed the maximum amount of objects created of type _E  !
                constexpr ERHIChild type = _E;
                INFLUX_GRAPHICS_ASSERT(false);

                return RHIObjectHandle<_E>{};
            }

            return TryRegisterRHIObject<_E>(creator());
        }
        template <class _E>
        static _E CreateAndRegisterRHIObject(Function<void*()> creator)
        {
            return CreateAndRegisterRHIObject<_E::GetStaticType()>(creator);
        }

        /* Get all RHI Objects registered based on the ERHIObject enum */
        template <ERHIChild _E>
        static const ChildContainer& GetRHIObjectsOfType()
        {
            return Get().m_childLists[static_cast<uint8>(_E)];
        }
        /* Get all RHI Objects based on their type */
        template <class _E>
        static const ChildContainer& GetRHIObjectsOfType()
        {
            static_assert(std::is_base_of_v<IRHIObjectHandle, _E> == true, "GetRHIObjectsOfType [_E] must derive from IRHIObjectHandle!");

            return GetRHIObjectsOfType<_E::GetStaticType()>();
        }

        /* Has a single RHIObject of type _E */
        template <ERHIChild _E>
        static bool HasObjectOfType()
        {
            return GetRHIObjectsOfType<_E>().size() != 0u;
        }
        template <class _E>
        static bool HasObjectOfType()
        {
            return HasObjectOfType<_E::GetStaticType()>();
        }

        /* GetRHIObjectsOfType[idx]*/
        template <ERHIChild _E>
        static RHIObjectHandle<_E> GetRHIObjectOfType(uint64 atIndex)
        {
            if (atIndex >= GetRHIObjectsOfType<_E>().size())
            {
                return RHIObjectHandle<_E>::GetInvalid();
            }

            constexpr uint64 typeIndex = static_cast<uint8>(_E);
            const IRHIObjectHandle* handle = Get().m_childLists[typeIndex][atIndex].pHandle;

            // Copy over the raw-pointer.
            return RHIObjectHandle<_E>{handle->GetInternal()};
        }
        template <class _E>
        static _E GetRHIObjectOfType(uint64 atIndex)
        {
            return GetRHIObjectOfType<_E::GetStaticType()>(atIndex);
        }

        /* Get Child (handle + state) */
        template <ERHIChild _E>
        static Child* FindChildFromHandle(const RHIObjectHandle<_E>& handle)
        {
#if INFLUX_GRAPHICS_USE_STL
            ChildContainer& containerToSearch = Get().m_childLists[static_cast<uint8>(_E)];
            auto found = std::find_if(containerToSearch.begin(), containerToSearch.end(), 
            [&handle](const Child& child) -> bool
            {
                return child.pHandle->GetInternal() == handle.GetInternal();
            });

            if (found != containerToSearch.end())
            {
                return &(*found);
            }
#endif
            return nullptr;
        }
        static Child* FindChildFromHandle(const IRHIObjectHandle& handle)
        {
            Child* pointerToChild = nullptr;
            for (uint8 c = 0u; c < k_numRHIObjectTypes; ++c)
            {
#if INFLUX_GRAPHICS_USE_STL
                ChildContainer& containerToSearch = Get().m_childLists[c];
                auto found = std::find_if(containerToSearch.begin(), containerToSearch.end(),
                [&handle](const Child& child) -> bool
                {
                    return child.pHandle->GetInternal() == handle.GetInternal();
                });
#endif
            }

            return pointerToChild;
        }

        /* Get & set RHI State */
        template <ERHIChild _E>
        static void SetRHIState(const RHIObjectHandle<_E>& handle, const RHIState<_E>& newState)
        {
            if (Child* child = GetChildFromHandle<_E>(handle))
            {
                child->State = newState;
            }
        }
        template <ERHIChild _E>
        static const RHIState<_E>* GetRHIState(const RHIObjectHandle<_E>& handle)
        {
            if (const Child* child = GetChildFromHandle<_E>(handle))
            {
                return &child->State;
            }

            return nullptr;
        }

    private:
        EGraphicsAPI m_currentInitializedAPI = EGraphicsAPI::NotSupported;
        bool m_isDebugLayerActive = false;

        Array<ChildContainer, static_cast<uint8>(ERHIChild::Max)> m_childLists;

#if INFLUX_GRAPHICS_INCLUDE_DX12
    private:
        Vector<ID3D12Device*> DxLogicalDevices;
        Vector<IDXGIAdapter*> DxgiPhysicalDevices;
        IDXGIFactory2* DxgiFactory2;

        uint32 MainAdapterIndex;

        struct DescriptorHeap
        {
            RHIDescriptorHeapHandle RHIHandle;
        };

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

            RHIDescriptorHeapDesc desc{};

            desc.Type = ERHIResourceViewType::RTV;
            CreateDescriptorHeap(desc, m_globalRtvDescriptorHeap);

            desc.Type = ERHIResourceViewType::DSV;
            CreateDescriptorHeap(desc, m_globalDsvDescriptorHeap);

            desc.Type = ERHIResourceViewType::Sampler;
            CreateDescriptorHeap(desc, m_globalSamplerDescriptorHeap);

            desc.Type = ERHIResourceViewType::Resource;
            CreateDescriptorHeap(desc, m_globalResDescriptorHeap);

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

        static const RHIDescriptorHeapHandle& GetGlobalRtvHeap()
        {
            return Get().m_globalRtvDescriptorHeap;
        }

        uint64 DxCachedRtvDescriptorSize = 0;
        uint64 DxCachedDsvDescriptorSize = 0;
        uint64 DxCachedResourceDescriptorSize = 0;
        uint64 DxCachedSamplerDescriptorSize = 0;

        constexpr static uint8 k_dxMaxNumSamplerDescriptorsPerHeap = 16u;
        constexpr static uint8 k_dxMaxNumResourceDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumRtvDescriptorsPerHeap = 64u;
        constexpr static uint8 k_dxMaxNumDsvDescriptorsPerHeap = 64u;

        RHIDescriptorHeapHandle m_globalRtvDescriptorHeap;
        RHIDescriptorHeapHandle m_globalDsvDescriptorHeap;
        RHIDescriptorHeapHandle m_globalResDescriptorHeap;
        RHIDescriptorHeapHandle m_globalSamplerDescriptorHeap;

        struct CommandAllocatorEntry final
        {
            RHIGraphicsCommandBufferHandle Handle;
            uint64 FenceValue;
            
            bool GetIsInUse(uint64 fenceValue)
            {
                return this->FenceValue >= fenceValue;
            }
        };

        // Call this when 1 or more command lists with this command allocator get submitted onto the gpu.
        static void OnLaunchCommandAllocator(RHIGraphicsCommandBufferHandle handle, uint64 fenceValue)
        {
            CommandAllocatorEntry newEntry{};

            newEntry.FenceValue = fenceValue;
            newEntry.Handle = handle;

            Get().m_commandAllocatorList.push_back(newEntry);
        }

        constexpr static uint32 k_maxNumCommandAllocators = k_maxNumRHIObjectsPerType[static_cast<uint32>(ERHIChild::CommandAllocator)];
        CommandAllocatorEntry m_allCommandAllocators[k_maxNumCommandAllocators];
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN

#endif
    };


    EResult RegisterNative(EGraphicsAPI api, ERHIChild type, void* ptr)
    {
        switch (type)
        {
            case ERHIChild::CommandQueue:           GlobalState::TryRegisterRHIObject<ERHIChild::CommandQueue>(ptr); break;
            case ERHIChild::CommandList:            GlobalState::TryRegisterRHIObject<ERHIChild::CommandList>(ptr); break;
            case ERHIChild::CommandAllocator:       GlobalState::TryRegisterRHIObject<ERHIChild::CommandAllocator>(ptr); break;
            case ERHIChild::Swapchain:              GlobalState::TryRegisterRHIObject<ERHIChild::Swapchain>(ptr); break;
            case ERHIChild::GraphicsPipeline:       GlobalState::TryRegisterRHIObject<ERHIChild::GraphicsPipeline>(ptr); break;
            case ERHIChild::GraphicsPipelineLayout: GlobalState::TryRegisterRHIObject<ERHIChild::GraphicsPipelineLayout>(ptr); break;
            case ERHIChild::Texture:                GlobalState::TryRegisterRHIObject<ERHIChild::Texture>(ptr); break;
            case ERHIChild::Buffer:                 GlobalState::TryRegisterRHIObject<ERHIChild::Buffer>(ptr); break;
            case ERHIChild::DescriptorHeap:         GlobalState::TryRegisterRHIObject<ERHIChild::DescriptorHeap>(ptr); break;
            case ERHIChild::Descriptor:             GlobalState::TryRegisterRHIObject<ERHIChild::Descriptor>(ptr); break;
        }

        return { true };
    }

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

    /* Creates a Graphics command buffer OR gets one that is no longer in use */
    EResult GetGraphicsCommandBuffer(RHIGraphicsCommandBufferHandle& out_handle)
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
        INFLUX_GRAPHICS_ASSERT(out_existingCommandBuffer.IsValid());

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIGraphicsCommandListHandle>([&out_existingCommandBuffer]()
            {
                ID3D12CommandAllocator* cmdAllocator = out_existingCommandBuffer.As<ID3D12CommandAllocator>();

                ID3D12GraphicsCommandList* gfxCommandList = D3D12::CreateDxCommandList(
                    GlobalState::GetDevice(), 
                    cmdAllocator,
                    D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

                // So we can query the allocator from the command list ;)
                gfxCommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), cmdAllocator);

                /* Get the private held Command Allocator */
                ID3D12CommandAllocator* cmdAllocator;
                UINT dataSize = sizeof(cmdAllocator);
                dxCmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &cmdAllocator);

                return gfxCommandList;
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

            ID3D12GraphicsCommandList* d3d12CommandList = commandListHandle.As<ID3D12GraphicsCommandList>();
            ID3D12CommandAllocator* d3d12Allocator = commandbufferHandle.As<ID3D12CommandAllocator>();

            d3d12CommandList->Close();

            d3d12CommandList->Reset(d3d12Allocator, nullptr);

            return { true };
            break;
#endif
        }

        INFLUX_GRAPHICS_ASSERT(false);
        return { false };
    }

    EResult DispatchGraphicsCommandListToGpu(const RHIGraphicsCommandListHandle& commandListHandle, const RHIGraphicsCommandQueueHandle& commandQueueHandle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            ID3D12GraphicsCommandList* d3d12GfxCmdList = commandListHandle.As<ID3D12GraphicsCommandList>();
            INFLUX_GRAPHICS_ASSERT(d3d12GfxCmdList != nullptr);

            ID3D12CommandQueue* d3d12CommandQueue = commandQueueHandle.As<ID3D12CommandQueue>();
            INFLUX_GRAPHICS_ASSERT(d3d12CommandQueue != nullptr);

            // Close the command list...
            d3d12GfxCmdList->Close();

            ID3D12CommandList* d3d12CmdLists[1u]{ d3d12GfxCmdList };
            d3d12CommandQueue->ExecuteCommandLists(1u, d3d12CmdLists);

            return {};

            break;
#endif
        }

        INFLUX_GRAPHICS_TODO;
        return { false };
    }

    EResult CreateSwapchain(const RHISwapchainDesc& desc, RHISwapchainHandle& out_handle)
    {
        uint8 numBuffers = desc.GetNumBuffers();

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
                out_handle.GetInternal<IDXGISwapChain3>()->GetBuffer(i, IID_PPV_ARGS(&resource));
                if (resource != nullptr)
                {
                    GlobalState::TryRegisterRHIObject<RHIBufferHandle>(resource);
                }
                else INFLUX_GRAPHICS_ASSERT(false);
            }

            break;
#endif
        }

        return EResult(true);
    }

    EResult DispatchSwapchainPresent(const RHISwapchainHandle& swapchain, const PresentDescription& present)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            swapchain.As<IDXGISwapChain>()->Present(present.Vsync ? 1u : 0u, 0);
            break;
#endif
        }

        return { false };
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

    EResult CreateRenderTargetView(const RHIDescriptorHeapHandle& descriptorHeap, const RHIBufferHandle& bufferHandle, RHIDescriptorHandle& out_handle)
    {
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:

            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIDescriptorHandle>([&bufferHandle, &descriptorHeap]()
            {
                ID3D12Resource* d3d12Resource             = bufferHandle.As<ID3D12Resource>();
                ID3D12DescriptorHeap* d3d12DescriptorHeap = descriptorHeap.As<ID3D12DescriptorHeap>();

                const uint64 offsetInHeap = 0u;
                D3D12_CPU_DESCRIPTOR_HANDLE offsettedCpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE(d3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + offsetInHeap);

                GlobalState::GetDevice()->CreateRenderTargetView(d3d12Resource, nullptr, offsettedCpuHandle);

                return reinterpret_cast<void*>(offsettedCpuHandle.ptr);
            });

            break;
#endif
        }

        return EResult(true);
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

        // Memory allocator for all the commands...
        RHIGraphicsCommandBufferHandle cmdBufferHandle;
        GetGraphicsCommandBuffer(cmdBufferHandle);
        INFLUX_GRAPHICS_ASSERT(cmdBufferHandle);

        // List of commands we'll pass on for the entire 'frame'
        RHIGraphicsCommandListHandle cmdListHandle;
        CreateGraphicsCommandList(cmdBufferHandle, cmdListHandle);
        INFLUX_GRAPHICS_ASSERT(cmdListHandle);
        
        if (cmdListHandle.IsValid() && cmdBufferHandle.IsValid())
        {
            // Record commands...
            commands(cmdListHandle);

            GlobalState::OnLaunchCommandAllocator(cmdBufferHandle, 0u);

            DispatchGraphicsCommandListToGpu(cmdListHandle, cmdQueueHandle);
        }
        
        return EResult();
    }

    namespace Commands
    {
        EResult ClearRenderTargetView(const RHIGraphicsCommandListHandle& cmdListHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            if (ID3D12GraphicsCommandList* d3d12CmdList = cmdListHandle.As<ID3D12GraphicsCommandList>())
            {
                // d3d12CmdList->ClearRenderTargetView()
            }
#endif

            return { true };
        }

        EResult ClearSwapchainBackBuffer(const RHIGraphicsCommandListHandle& cmdListHandle, const RHISwapchainHandle& swapchainHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            ID3D12GraphicsCommandList* gfxCmdList = cmdListHandle.As<ID3D12GraphicsCommandList>();
            INFLUX_GRAPHICS_ASSERT(gfxCmdList);

            IDXGISwapChain3* swapchain = swapchainHandle.As<IDXGISwapChain3>();
            INFLUX_GRAPHICS_ASSERT(swapchain);
            
            ID3D12Resource* bufferResource;
            swapchain->GetBuffer(swapchain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&bufferResource));
            RHIBufferHandle bufferRHIHandle = GlobalState::TryRegisterRHIObject<RHIBufferHandle>(bufferResource);

            const RHIDescriptorHandle& rtvRHIHandle{};
            // CreateRenderTargetView(GlobalState::GetGlobalRtvHeap(), bufferRHIHandle, rtvRHIHandle);

            // gfxCmdList->ClearRenderTargetView(rtvRHIHandle.As<, colour.data, 0u, nullptr);

            // Do we need to?
            bufferResource->Release();
#endif

            return { true };
        }
    }
}