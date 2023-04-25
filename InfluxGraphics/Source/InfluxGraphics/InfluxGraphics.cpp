#include "InfluxGraphics.h"

#if INFLUX_GRAPHICS_USE_CORE
#include "Core/Singleton/Singleton.h"
#else
static_assert(false, "No Singleton Implementation without Influx::Core...");
#endif

#if INFLUX_GRAPHICS_INCLUDE_DX12
#include "InfluxGraphics/D3D12/D3D12.h"
#include "InfluxGraphics/D3D12/D3D12Conversion.h"
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN
#include "InfluxGraphics/Vulkan/Vulkan.h"
#endif

#ifndef __TODO
#define __TODO INFLUX_GRAPHICS_TODO
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
        };

        using ChildContainer = Vector<Child>;

        struct CommandQueueEntry final
        {
            RHICommandQueueHandle Handle = nullptr;

#if INFLUX_GRAPHICS_INCLUDE_DX12
            ID3D12Fence* pDxFence = nullptr;
#endif
        };

        struct DescriptorHeapEntry final
        {
            RHIDescriptorHeapHandle Handle;
        };

        struct CommandBufferEntry final
        {
            RHICommandBufferHandle Handle;
            uint64 FenceValue;

            bool GetIsInFlight(uint64 fenceValue) const
            {
                return (this->FenceValue < fenceValue);
            }
        };

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
        
#pragma region RHI Object Registering
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
            RHIObjectHandle<_E>* newHandle  = new RHIObjectHandle<_E>(rawObjectPointer);

            Child newChild{};
            newChild.pHandle    = newHandle;

            // Handle special cases.
            switch (_E)
            {
            case ERHIChild::DescriptorHeap:
                break;

            case ERHIChild::CommandQueue:
                OnRegisterCommandQueue(newHandle);
                break;
            }

            // Add child to list.
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

            void* creationResult = creator();
            INFLUX_GRAPHICS_ASSERT(creationResult != nullptr);

            return TryRegisterRHIObject<_E>(creationResult);
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
#else
            static_assert(false, "FindChildFromHandle() no implementation withtout STL library...");
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
#else
                static_assert(false, "FindChildFromHandle() no implementation withtout STL library...");
#endif
            }

            return pointerToChild;
        }
#pragma endregion

    private:
        EGraphicsAPI m_currentInitializedAPI = EGraphicsAPI::NotSupported;
        bool m_isDebugLayerActive = false;
        uint32 m_mainAdapterIndex;

        // All RHI child objects:
        Array<ChildContainer, static_cast<uint8>(ERHIChild::Max)> m_childLists;

        // Global RHI objects:
        DescriptorHeapEntry m_globalRtvDescriptorHeap;
        DescriptorHeapEntry m_globalDsvDescriptorHeap;
        DescriptorHeapEntry m_globalResDescriptorHeap;
        DescriptorHeapEntry m_globalSamplerDescriptorHeap;

        constexpr static uint8 k_numCommandBuffers = GetMaxNumOfObjects(ERHIChild::CommandBuffer);
        CommandBufferEntry m_allCommandBuffers[k_numCommandBuffers];

        CommandQueueEntry m_globalGraphicsCommandQueue;
        CommandQueueEntry m_globalCopyCommandQueue;
        CommandQueueEntry m_globalComputeCommandQueue;

    public:
        static CommandQueueEntry& GetGlobalGraphicsCommandQueue()
        {
            return Get().m_globalGraphicsCommandQueue;
        }

        static const CommandQueueEntry& GetGlobalCopyCommandQueue()
        {
            return Get().m_globalComputeCommandQueue;
        }

        static const CommandQueueEntry& GetGlobalComputeCommandQueue()
        {
            return Get().m_globalComputeCommandQueue;
        }

        static const DescriptorHeapEntry& GetGlobalRtvHeap()
        {
            return Get().m_globalRtvDescriptorHeap;
        }

        static RHICommandBufferHandle GetAvailableGraphicsCommandBuffer(uint64 readyAtFenceValue)
        {
            RHICommandBufferHandle out_handle{};

            for (uint8 i = 0u; i < k_numCommandBuffers; ++i)
            {
                const CommandBufferEntry& commandBuffer = Get().m_allCommandBuffers[i];

                if (!commandBuffer.GetIsInFlight(readyAtFenceValue))
                {
                    return commandBuffer.Handle;
                }
            }

            return out_handle;
        }
    private:
        static void OnRegisterCommandQueue(const RHICommandQueueHandle& handle)
        {
           Get().m_globalGraphicsCommandQueue.Handle = handle;
        }

#if INFLUX_GRAPHICS_INCLUDE_DX12
    private:
        Vector<ID3D12Device*>   DxLogicalDevices;
        Vector<IDXGIAdapter*>   DxgiPhysicalDevices;
        IDXGIFactory2*          DxgiFactory2;

    public:
        Result InitializeDx12()
        {
            m_currentInitializedAPI = EGraphicsAPI::D3D12;

            DxgiFactory2 = D3D12::Factory::CreateTier2(m_isDebugLayerActive);
            DxgiPhysicalDevices = D3D12::Adapter::SelectAll(DxgiFactory2);
            m_mainAdapterIndex = 0u; // Temp...

            for (uint64 i = 0u; i < DxgiPhysicalDevices.size(); ++i)
            {
                if (i == m_mainAdapterIndex)
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

            Result result{};

            // Create global Descriptor heaps...
            desc.Type = ERHIResourceViewType::RTV;
            result = CreateDescriptorHeap(desc, m_globalRtvDescriptorHeap.Handle);

            desc.Type = ERHIResourceViewType::DSV;
            result = CreateDescriptorHeap(desc, m_globalDsvDescriptorHeap.Handle);

            desc.Type = ERHIResourceViewType::Sampler;
            result = CreateDescriptorHeap(desc, m_globalSamplerDescriptorHeap.Handle);

            desc.Type = ERHIResourceViewType::Resource;
            result = CreateDescriptorHeap(desc, m_globalResDescriptorHeap.Handle);

            // Create global command queues...
            result = CreateGraphicsCommandQueue(m_globalGraphicsCommandQueue.Handle);
            result = CreateComputeCommandQueue(m_globalComputeCommandQueue.Handle);

            // Create global command buffers...
            for (uint8 i = 0u; i < GetMaxNumOfObjects(Graphics::ERHIChild::CommandBuffer); ++i)
            {
                m_allCommandBuffers[i].FenceValue = 0u;
                result = CreateGraphicsCommandBuffer(m_allCommandBuffers[i].Handle);
            }
            
            return Result(true);
        }

        static Result CleanupDx12()
        {
            for (uint64 i = 0u; i < Get().DxgiPhysicalDevices.size(); ++i)
            {
                if (Get().DxLogicalDevices[i] != nullptr)
                {
                    Get().DxLogicalDevices[i]->Release();
                    Get().DxLogicalDevices[i] = nullptr;
                }
                if (Get().DxgiPhysicalDevices[i] != nullptr)
                {
                    Get().DxgiPhysicalDevices[i]->Release();
                    Get().DxgiPhysicalDevices[i] = nullptr;
                }
            }

            Get().DxgiFactory2->Release();
            return Result(true);
        }

        static ID3D12Device* GetDevice()
        {
            return Get().DxLogicalDevices[Get().m_mainAdapterIndex];
        }

        static IDXGIFactory2* GetFactory2()
        {
            return Get().DxgiFactory2;
        }

        uint64 DxCachedRtvDescriptorSize        = 0u;
        uint64 DxCachedDsvDescriptorSize        = 0u;
        uint64 DxCachedResourceDescriptorSize   = 0u;
        uint64 DxCachedSamplerDescriptorSize    = 0u;
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN

        Result InitializeVulkan()
        {
            INFLUX_GRAPHICS_TODO;
            return Result{ false };
        }

        Result CleanupVulkan()
        {
            INFLUX_GRAPHICS_TODO;
            return Result{ false };
        }
#endif
    };

    Result RegisterNative(EGraphicsAPI api, ERHIChild type, void* ptr)
    {
        switch (type)
        {
            case ERHIChild::CommandQueue:           GlobalState::TryRegisterRHIObject<ERHIChild::CommandQueue>(ptr); break;
            case ERHIChild::CommandList:            GlobalState::TryRegisterRHIObject<ERHIChild::CommandList>(ptr); break;
            case ERHIChild::CommandBuffer:          GlobalState::TryRegisterRHIObject<ERHIChild::CommandBuffer>(ptr); break;
            case ERHIChild::Swapchain:              GlobalState::TryRegisterRHIObject<ERHIChild::Swapchain>(ptr); break;
            case ERHIChild::GraphicsPipeline:       GlobalState::TryRegisterRHIObject<ERHIChild::GraphicsPipeline>(ptr); break;
            case ERHIChild::GraphicsPipelineLayout: GlobalState::TryRegisterRHIObject<ERHIChild::GraphicsPipelineLayout>(ptr); break;
            case ERHIChild::Resource:               GlobalState::TryRegisterRHIObject<ERHIChild::Resource>(ptr); break;
            case ERHIChild::DescriptorHeap:         GlobalState::TryRegisterRHIObject<ERHIChild::DescriptorHeap>(ptr); break;
            case ERHIChild::Descriptor:             GlobalState::TryRegisterRHIObject<ERHIChild::Descriptor>(ptr); break;
        }

        return { true };
    }

    Result Initialize(EGraphicsAPI api)
    {
        Result result{ false };

        if (GlobalState::HasInitializedGraphicsAPI())
        {
            result = Cleanup();
        }

        switch (api)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            result = GlobalState::Get().InitializeDx12();
            break;

#elif INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            result = GlobalState::Get().InitializeVulkan();
            break;
#endif
        }

        return result;
    }

    Result Cleanup()
    {
        if (!GlobalState::Get().HasInitializedGraphicsAPI())
        {
            return Result(false);
        }

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            return GlobalState::Get().CleanupDx12();
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            return GlobalState::Get().CleanupVulkan();
#endif
        }

        return Result();
    }

    Result Create(EGraphicsAPI api, Function<void()> internalFunc)
    {
        Result result{};

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

    Result SetDebugLayerEnabled()
    {
        GlobalState::SetDebugLayerActive(true);

#if INFLUX_GRAPHICS_INCLUDE_DX12
        D3D12::EnableDxDebugLayer();
#endif

        return Result{true};
    }

    bool IsDebugLayerEnabled()
    {
        return GlobalState::IsDebugLayerActive();
    }

    /* Creates a Graphics command queue OR gets an existing one */
    Result GetGraphicsCommandQueue(RHICommandQueueHandle& out_handle)
    {
        Result result{};

        out_handle = GlobalState::GetGlobalGraphicsCommandQueue().Handle;
        INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());

        return result;
    }

    /* Creates a Compute command queue OR gets an existing one */
    Result GetComputeCommandQueue(RHICommandQueueHandle& out_handle)
    {
        Result result{};

        out_handle = GlobalState::GetGlobalComputeCommandQueue().Handle;
        INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());

        return result;
    }

    Result CreateComputeCommandQueue(RHICommandQueueHandle& out_handle)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandQueueHandle>([]()
            {
                return D3D12::CreateDxCommandQueue(GlobalState::GetDevice(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE);
            });

            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandQueueHandle>([]()
            {
                __TODO;
                return nullptr;
            });

            break;
#endif
        }

        return result;
    }

    Result CreateGraphicsCommandQueue(RHICommandQueueHandle& out_handle)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandQueueHandle>([]()
            {
                ID3D12CommandQueue* resultCommandQueue = D3D12::CreateDxCommandQueue(GlobalState::GetDevice(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);;
                return resultCommandQueue;
            });
            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandQueueHandle>([]()
                {
                    __TODO;
                    return nullptr;
                });

            break;
#endif
        }

        return result;
    }

    Result GetGraphicsCommandBuffer(RHICommandBufferHandle& out_handle, uint64 fenceValue)
    {
        Result result{};

        out_handle = GlobalState::GetAvailableGraphicsCommandBuffer(fenceValue);
        INFLUX_GRAPHICS_ASSERT(out_handle);

        // Resetting it...
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle.As<ID3D12CommandAllocator>()->Reset();
            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }


        return result;
    }

    Result CreateComputeCommandBuffer(RHICommandBufferHandle& out_handle)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandBufferHandle>([]()
            {
                return D3D12::CreateDxCommandAllocator(GlobalState::GetDevice(),
                    D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE);
            });
            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandBufferHandle>([]()
                {
                    __TODO;
                    return nullptr;
                });

            break;
#endif
        }

        return result;
    }

    Result CreateGraphicsCommandBuffer(RHICommandBufferHandle& out_handle)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandBufferHandle>([]()
            {
                return D3D12::CreateDxCommandAllocator(GlobalState::GetDevice(),
                    D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
            });

            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandBufferHandle>([]()
            {
                __TODO;
                return nullptr;
            });

            break;
#endif
        }

        return result;
    }

    Result CreateComputeCommandList(RHICommandBufferHandle& out_existingCommandBuffer, RHICommandListHandle& out_handle)
    {
        Result result{};

        INFLUX_GRAPHICS_ASSERT(out_existingCommandBuffer.IsValid());

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandListHandle>([&out_existingCommandBuffer]()
            {
                ID3D12CommandAllocator* d3d12Allocator = out_existingCommandBuffer.As<ID3D12CommandAllocator>();
                ID3D12CommandList* gfxCommandList = D3D12::CreateDxCommandList(
                    GlobalState::GetDevice(),
                    d3d12Allocator,
                    D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE);

                // So we can query the allocator from the command list ;)
                gfxCommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), d3d12Allocator);

                return gfxCommandList;
            });

            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }
        return result;
    }

    Result CreateGraphicsCommandList(RHICommandBufferHandle& out_existingCommandBuffer, RHICommandListHandle& out_handle)
    {
        INFLUX_GRAPHICS_ASSERT(out_existingCommandBuffer.IsValid());

        if (!GlobalState::CanRegisterRHIObject<RHICommandListHandle>())
        {
            INFLUX_GRAPHICS_ASSERT(false);
            return Result(false);
        }

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHICommandListHandle>([&out_existingCommandBuffer]()
            {
                ID3D12CommandAllocator* cmdAllocator = out_existingCommandBuffer.As<ID3D12CommandAllocator>();

                ID3D12CommandList* gfxCommandList = D3D12::CreateDxCommandList(
                    GlobalState::GetDevice(), 
                    cmdAllocator,
                    D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

                // So we can query the allocator from the command list ;)
                gfxCommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), cmdAllocator);

                /* Get the private held Command Allocator */
                UINT dataSize = sizeof(cmdAllocator);
                gfxCommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &cmdAllocator);

                return gfxCommandList;
            });

            break;
#elif INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIGraphicsCommandListHandle>([&out_existingCommandBuffer]()
            {
                __TODO;
                return nullptr;
            });
            break;
#endif
        }

        // Reset Graphics Command List with the associated Command Buffer...
        ResetGraphicsCommandlist(out_handle, out_existingCommandBuffer);

        return Result(true);
    }

    Result ResetGraphicsCommandlist(const RHICommandListHandle& commandListHandle, const RHICommandBufferHandle& commandbufferHandle)
    {
        Result result{};

        INFLUX_GRAPHICS_ASSERT(commandListHandle.IsValid());
        INFLUX_GRAPHICS_ASSERT(commandbufferHandle.IsValid());

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
        {
            ID3D12GraphicsCommandList* d3d12CommandList = commandListHandle.As<ID3D12GraphicsCommandList>();
            ID3D12CommandAllocator* d3d12Allocator = commandbufferHandle.As<ID3D12CommandAllocator>();

            d3d12CommandList->Close();

            d3d12CommandList->Reset(d3d12Allocator, nullptr);
        }
        break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }

        return result;
    }

    /* Waits for the global graphics command queue to reach signal-value */
    Result WaitForGraphicsQueueSignal(const RHICommandQueueHandle& commandQueue, uint64 valueToWaitFor)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
        {
            ID3D12Fence* fence = GlobalState::GetGlobalGraphicsCommandQueue().pDxFence;
            const uint64 fenceCompletedValue = fence->GetCompletedValue();

            // Our fence completed value exceeds our value to wait for. This means the value we're waiting for has been exceeded in the past!
            if (fenceCompletedValue >= valueToWaitFor)
            {
                return result;
            }

            // Wait for the completion event:
            ::HANDLE completionEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
            INFLUX_GRAPHICS_ASSERT(fence->SetEventOnCompletion(valueToWaitFor, completionEvent));
            ::WaitForSingleObjectEx(completionEvent, INFINITE, FALSE);
        } break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            // vk::SemaphoreWaitInfo wait_info = {};
            // wait_info.semaphoreCount = 1;
            // wait_info.pSemaphores = &m_timeline_semaphore.get();
            // wait_info.pValues = &value;
            // m_device.GetDevice().waitSemaphoresKHR(wait_info, UINT64_MAX);
            __TODO;
            break;
#endif
        }

        return result;
    }

    /* Waits for the global graphics command queue to finish ALL work */
    Result WaitForAllGraphicsCommandsFinished()
    {
        Result result{};

        __TODO;

        return result;
    }

    Result DispatchComputeCommandListToGpu(const RHICommandListHandle& commandListHandle, const RHICommandQueueHandle& commandQueueHandle)
    {
        Result result{};

        INFLUX_GRAPHICS_ASSERT(commandListHandle.IsValid());
        INFLUX_GRAPHICS_ASSERT(commandQueueHandle.IsValid());

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
        {
            ID3D12GraphicsCommandList* d3d12CmdList = commandListHandle.As<ID3D12GraphicsCommandList>();
            INFLUX_GRAPHICS_ASSERT(d3d12CmdList != nullptr);

            ID3D12CommandQueue* d3d12CommandQueue = commandQueueHandle.As<ID3D12CommandQueue>();
            INFLUX_GRAPHICS_ASSERT(d3d12CommandQueue != nullptr);
            
            d3d12CmdList->Close();

            // Close the command list...
            ID3D12CommandList* d3d12CmdLists[1u]{ d3d12CmdList };
            d3d12CommandQueue->ExecuteCommandLists(1u, d3d12CmdLists);
        }
        break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }

        return result;
    }

    Result DispatchGraphicsCommandListToGpu(const RHICommandListHandle& commandListHandle, const RHICommandQueueHandle& commandQueueHandle)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
        {
            ID3D12GraphicsCommandList* d3d12GfxCmdList = commandListHandle.As<ID3D12GraphicsCommandList>();
            INFLUX_GRAPHICS_ASSERT(d3d12GfxCmdList != nullptr);

            ID3D12CommandQueue* d3d12CommandQueue = commandQueueHandle.As<ID3D12CommandQueue>();
            INFLUX_GRAPHICS_ASSERT(d3d12CommandQueue != nullptr);

            d3d12GfxCmdList->RSSetViewports(0u, nullptr);

            // Close the command list...
            d3d12GfxCmdList->Close();

            ID3D12CommandList* d3d12CmdLists[1u]{ d3d12GfxCmdList };
            d3d12CommandQueue->ExecuteCommandLists(1u, d3d12CmdLists);
            break;
        }
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }

        return result;
    }

    Result CreateSwapchain(const RHISwapchainDesc& desc, RHISwapchainHandle& out_handle)
    {
        Result result{};

        // Get an existing, or create a new graphics command queue...
        RHICommandQueueHandle cmdQueueHandle;
        result = GetGraphicsCommandQueue(cmdQueueHandle);
        INFLUX_GRAPHICS_ASSERT(cmdQueueHandle.IsValid());

        const uint8 numBuffers = desc.GetNumBuffers();
        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHISwapchainHandle>([&desc, &cmdQueueHandle, numBuffers]()
            {
                ID3D12CommandQueue* d3d12CommandQueue = cmdQueueHandle.As<ID3D12CommandQueue>();
                uint64 out_freq;
                d3d12CommandQueue->GetTimestampFrequency(&out_freq);

                return D3D12::Swapchain::CreateTier3(
                    GlobalState::GetFactory2(), 
                    (::HWND)desc.WindowHandle, 
                    d3d12CommandQueue,
                    desc.Dimensions.x, desc.Dimensions.y, 
                    numBuffers);
            });

            // Register buffer-resources to RHI:
            for (uint8 i = 0; i < numBuffers; ++i)
            {
                ID3D12Resource* resource;
                out_handle.GetInternal<IDXGISwapChain3>()->GetBuffer(i, IID_PPV_ARGS(&resource));
                INFLUX_GRAPHICS_ASSERT(resource != nullptr);

                GlobalState::TryRegisterRHIObject<RHIResourceHandle>(resource);
            }
            break;
#endif
        }

        INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());
        return result;
    }

    Result DispatchSwapchainPresent(const RHISwapchainHandle& swapchain, const PresentDescription& present)
    {
        Result result{};

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            swapchain.As<IDXGISwapChain>()->Present(present.Vsync ? 1u : 0u, 0);
            break;
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            __TODO;
            break;
#endif
        }

        return result;
    }

    Result CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& out_handle)
    {
        Result result{};

        D3D12_DESCRIPTOR_HEAP_TYPE type{};
        uint8 numDescriptors{};
        bool isShaderVisible = false;
        switch (desc.Type)
        {
        case ERHIResourceViewType::Resource:
            type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            numDescriptors = k_maxNumResourceDescriptorsPerHeap;
            isShaderVisible = true;
            break;

        case ERHIResourceViewType::DSV:
            type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            numDescriptors = k_maxNumDsvDescriptorsPerHeap;
            isShaderVisible = false;
            break;

        case ERHIResourceViewType::RTV:
            type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            numDescriptors = k_maxNumRtvDescriptorsPerHeap;
            isShaderVisible = false;
            break;

        case ERHIResourceViewType::Sampler:
            type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            numDescriptors = k_maxNumSamplerDescriptorsPerHeap;
            isShaderVisible = true;
            break;
        }

        switch (GetInitializedGraphicsAPI())
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
        case EGraphicsAPI::D3D12:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIDescriptorHeapHandle>([&desc, type, numDescriptors, isShaderVisible]()
            {
                return D3D12::CreateDxDescriptorHeap(GlobalState::GetDevice(), type, numDescriptors,
                    isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
            });
            break;
#endif

#if INFLUX_GRAPHICS_INCLUDE_VULKAN
        case EGraphicsAPI::Vulkan:
            out_handle = GlobalState::CreateAndRegisterRHIObject<RHIDescriptorHeapHandle>([&desc, type, numDescriptors, isShaderVisible]()
            {
                __TODO;
                return nullptr;
            });
            break;
#endif
        }

        return result;
    }

    /* Create a Descriptor heap OR gets on that has been created of ERHIDescriptorType */
    Result GetDescriptorHeap(const ERHIDescriptorType type, RHIDescriptorHeapHandle& out_handle)
    {
        Result result{};

        __TODO;

        return result;
    }

    Result CreateRenderTargetView(const RHIDescriptorHeapHandle& descriptorHeap, const RHIResourceHandle& bufferHandle, RHIDescriptorHandle& out_handle)
    {
        INFLUX_GRAPHICS_ASSERT(descriptorHeap.IsValid());
        INFLUX_GRAPHICS_ASSERT(bufferHandle.IsValid());

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

        INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());
        return Result(true);
    }

    /* Uses GetDescriptorHeap() for convenience */
    Result CreateRenderTargetView(const RHIResourceHandle& bufferHandle, RHIDescriptorHandle& out_handle)
    {
        Result result(false);

        RHIDescriptorHeapHandle descriptorHeap;
        result = GetDescriptorHeap(ERHIDescriptorType::RTV, descriptorHeap);
        INFLUX_GRAPHICS_ASSERT(descriptorHeap.IsValid());

        result = CreateRenderTargetView(descriptorHeap, bufferHandle, out_handle);
        INFLUX_GRAPHICS_ASSERT(out_handle.IsValid());

        return Result(true);
    }

    Result DispatchGraphicsCommands(Function<void(const RHICommandListHandle&)> commands)
    {
        Result result{};

        if (commands == nullptr)
        {
            return result;
        }

        // Get an existing, or create a new graphics command queue...
        RHICommandQueueHandle cmdQueueHandle;
        result = GetGraphicsCommandQueue(cmdQueueHandle);
        INFLUX_GRAPHICS_ASSERT(cmdQueueHandle.IsValid());

        // Memory allocator for all the commands...
        RHICommandBufferHandle cmdBufferHandle;
        result = GetGraphicsCommandBuffer(cmdBufferHandle, 0u);
        INFLUX_GRAPHICS_ASSERT(cmdBufferHandle.IsValid());

        // List of commands we'll pass on for the entire 'frame'
        RHICommandListHandle cmdListHandle;
        result = CreateGraphicsCommandList(cmdBufferHandle, cmdListHandle);
        INFLUX_GRAPHICS_ASSERT(cmdListHandle.IsValid());
        
        // Record commands...
        commands(cmdListHandle);

        result = DispatchGraphicsCommandListToGpu(cmdListHandle, cmdQueueHandle);
        
        return Result();
    }

    Result DispatchComputeCommands(Function<void(const RHICommandListHandle&)> commands)
    {
        Result result{};

        if (commands == nullptr)
        {
            return result;
        }

        RHICommandQueueHandle cmdQueueHandle;
        result = GetComputeCommandQueue(cmdQueueHandle);
        INFLUX_GRAPHICS_ASSERT(cmdQueueHandle.IsValid());

        // Memory allocator for all the commands...
        RHICommandBufferHandle cmdBufferHandle;
        result = CreateComputeCommandBuffer(cmdBufferHandle);
        INFLUX_GRAPHICS_ASSERT(cmdBufferHandle.IsValid());

        RHICommandListHandle cmdListHandle;
        result = CreateComputeCommandList(cmdBufferHandle, cmdListHandle);
        INFLUX_GRAPHICS_ASSERT(cmdListHandle);

        // Record commands...
        commands(cmdListHandle);

        result = DispatchComputeCommandListToGpu(cmdListHandle, cmdQueueHandle);

        return result;
    }

    namespace Commands
    {
        Result ClearRenderTargetView(const RHICommandListHandle& cmdListHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            if (ID3D12GraphicsCommandList* d3d12CmdList = cmdListHandle.As<ID3D12GraphicsCommandList>())
            {
                // d3d12CmdList->ClearRenderTargetView()
            }
#endif

            return { true };
        }

        Result ClearSwapchainBackBuffer(const RHICommandListHandle& cmdListHandle, const RHISwapchainHandle& swapchainHandle, const Math::Vectorf4& colour)
        {
#if INFLUX_GRAPHICS_INCLUDE_DX12
            ID3D12GraphicsCommandList* gfxCmdList = cmdListHandle.As<ID3D12GraphicsCommandList>();
            INFLUX_GRAPHICS_ASSERT(gfxCmdList);

            IDXGISwapChain3* swapchain = swapchainHandle.As<IDXGISwapChain3>();
            INFLUX_GRAPHICS_ASSERT(swapchain);
            
            ID3D12Resource* bufferResource;
            swapchain->GetBuffer(swapchain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&bufferResource));
            RHIResourceHandle bufferRHIHandle = GlobalState::TryRegisterRHIObject<RHIResourceHandle>(bufferResource);

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