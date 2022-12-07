#include "Application.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Renderer.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

inline LRESULT CALLBACK WndProc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) 
        return true;

    switch (uMsg)
    {
    case WM_SIZE:
        // Todo... resize!
        break;
    case WM_CLOSE:
        ::DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

#include <iostream>
namespace Influx::Application
{
	Application::Application(int argc, char** argv, const Settings& desc)
        : m_hasStarted{false}
        , m_shouldQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
	{
#if PLATFORM_WINDOWS
        m_windowHandle = WindowsPlatform::CreateWindow(
            static_cast<int>(m_currentSettings.InitWindowSize.x),
            static_cast<int>(m_currentSettings.InitWindowSize.y),
            Influx::ToWString(desc.Name), WndProc);
#endif
	}

    void Application::Run()
    {
        m_hasStarted = true;

        if (m_creationSettings.Type == EApplicationType::ImGuiApp)
            InitializeDx12ImGuiRenderer();

#if PLATFORM_WINDOWS
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
#if FLX_APP_KEEP_TIMING_STATS
            m_beforeFrame = Time::Now();
#endif

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // [UPDATE]
#if FLX_APP_KEEP_TIMING_STATS
            m_beforeUpdate = Time::Now();
#endif
            {
                OnUpdate();
            }
#if FLX_APP_KEEP_TIMING_STATS
            m_timeStats.AddValue<TimeStats::EStat::Update>(Time::MsBetween<float>(Time::Now(), m_beforeUpdate));
#endif

            if (m_creationSettings.Type == EApplicationType::ImGuiApp)
            {
                // [UI RENDER]
#if FLX_APP_KEEP_TIMING_STATS
                m_beforeUIRender = Time::Now();
#endif
                Dx12Renderer::FrameContext* frameCtx = nullptr;
                {
#if FLX_APP_RENDERER_D3D12
                    ImGui_ImplDX12_NewFrame();
#endif
                    ImGui_ImplWin32_NewFrame();
                    ImGui::NewFrame();

                    OnUIRender();

                    ImGui::ShowDemoWindow();

                    ImGui::Render();

#if FLX_APP_RENDERER_D3D12
                    // Wait for next Frame Resources:
                    {
                        uint64 nextFrameIndex = g_renderer.m_frame + 1u;
                        g_renderer.m_frame = nextFrameIndex;
                        frameCtx = &g_renderer.m_frameContexts[nextFrameIndex % k_numFramesInFlight];

                        UINT64 fenceValue = frameCtx->FenceValue;
                        if (fenceValue != 0) // means no fence was signaled
                        {
                            frameCtx->FenceValue = 0;
                            g_renderer.m_fence->SetEventOnCompletion(fenceValue, g_renderer.m_fenceEvent);

                            HANDLE waitableObjects[] = { g_renderer.m_fenceEvent };
                            DWORD numWaitableObjects = 1;

                            // Stall!
                            ::WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);
                        }
                    }

                    uint backbufferIndex = g_renderer.mp_dxgiSwapchain->GetCurrentBackBufferIndex();
                    frameCtx->CommandAllocator->Reset();

                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Transition.pResource = g_renderer.mp_targetResources[backbufferIndex];
                    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    g_renderer.mp_dxCommandList->Reset(frameCtx->CommandAllocator, NULL);
                    g_renderer.mp_dxCommandList->ResourceBarrier(1, &barrier);

                    g_renderer.mp_dxCommandList->OMSetRenderTargets(1u, &g_renderer.m_targetDescriptors[backbufferIndex], FALSE, NULL);
                    g_renderer.mp_dxCommandList->SetDescriptorHeaps(1u, &g_renderer.mp_dxSrvDescHeap);

                    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_renderer.mp_dxCommandList);
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                    g_renderer.mp_dxCommandList->ResourceBarrier(1u, &barrier);
                    g_renderer.mp_dxCommandList->Close();

                    g_renderer.mp_dxCommandQueue->ExecuteCommandLists(1u, (ID3D12CommandList* const*)&g_renderer.mp_dxCommandList);
#endif
                }

#if FLX_APP_KEEP_TIMING_STATS
                m_timeStats.AddValue<TimeStats::EStat::UIRender>(Time::MsBetween<float>(Time::Now(), m_beforeUIRender));
#endif
                // [PRESENT]
#if FLX_APP_KEEP_TIMING_STATS
                m_beforePresent = Time::Now();
#endif
                {
#if FLX_APP_RENDERER_D3D12
                    g_renderer.mp_dxgiSwapchain->Present(k_useVSync ? 1u : 0u, 0u);

                    uint64 fenceValue = g_renderer.m_fenceLastSignaledValue + 1u;
                    g_renderer.mp_dxCommandQueue->Signal(g_renderer.m_fence, fenceValue);
                    g_renderer.m_fenceLastSignaledValue = fenceValue;
                    frameCtx->FenceValue = fenceValue;
#endif
                }
#if FLX_APP_KEEP_TIMING_STATS
                m_timeStats.AddValue<TimeStats::EStat::Present>(Time::MsBetween<float>(Time::Now(), m_beforePresent));
#endif
            }
            
#if FLX_APP_KEEP_TIMING_STATS
            m_timeStats.AddValue<TimeStats::EStat::Frame>(Time::MsBetween<float>(Time::Now(), m_beforeFrame));
#endif
            ++m_frame;
        }
#endif
    }

    void Application::Quit()
    {
        m_shouldQuit = true;
    }

	Application::~Application()
	{

	}

    void Application::InitializeDx12ImGuiRenderer()
    {
        using namespace Influx::Graphics;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();

#if PLATFORM_WINDOWS
        ImGui_ImplWin32_Init(m_windowHandle);

#if FLX_APP_RENDERER_D3D12

#if FLX_APP_RENDERER_DEBUG
        D3D12::EnableDxDebugLayer();
#endif

        // Factory -> Adapter -> Device
        IDXGIFactory4* factory4 = D3D12::CreateDxgiFactory4();
        IDXGIAdapter4* adapter4 = D3D12::GetDxgiAdapter4(factory4, k_useWarp);
        g_renderer.mp_dxDevice2 = D3D12::CreateDxDevice2(adapter4);

        // CmdQueue
        constexpr D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
        g_renderer.mp_dxCommandQueue = D3D12::CreateDxCommandQueue(g_renderer.mp_dxDevice2, cmdListType);

        // Desc-heaps
        g_renderer.mp_dxSrvDescHeap = D3D12::CreateDxDescriptorHeap(g_renderer.mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1u, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        g_renderer.mp_dxRtvDescHeap = D3D12::CreateDxDescriptorHeap(g_renderer.mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, k_numFramesInFlight);
        SIZE_T rtvDescriptorSize = g_renderer.mp_dxDevice2->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_renderer.mp_dxRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
        {
            g_renderer.m_targetDescriptors[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }

        // Allocators + Cmdlist
        for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
            g_renderer.m_frameContexts[i].CommandAllocator = D3D12::CreateDxCommandAllocator(g_renderer.mp_dxDevice2, cmdListType);

        g_renderer.mp_dxCommandList = D3D12::CreateDxCommandList(g_renderer.mp_dxDevice2,
            g_renderer.m_frameContexts[0].CommandAllocator, cmdListType);
        g_renderer.mp_dxCommandList->Close();

        // Fence
        g_renderer.m_fence = D3D12::CreateDxFence(g_renderer.mp_dxDevice2);
        g_renderer.m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        // Swapchain
        g_renderer.mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(factory4, m_windowHandle, g_renderer.mp_dxCommandQueue,
            m_currentSettings.InitWindowSize.x, m_currentSettings.InitWindowSize.y, k_numFramesInFlight);

        ImGui_ImplDX12_Init(g_renderer.mp_dxDevice2, k_numFramesInFlight,
            DXGI_FORMAT_R8G8B8A8_UNORM, g_renderer.mp_dxSrvDescHeap,
            g_renderer.mp_dxSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            g_renderer.mp_dxSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
#endif

#endif

        // Call this to resize the rendertargets
        ResizeWindow();
    }

    void Application::ResizeWindow()
    {
#if FLX_APP_RENDERER_D3D12
        for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
        {
            ID3D12Resource* backbuffer = nullptr;
            g_renderer.mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer));
            g_renderer.mp_dxDevice2->CreateRenderTargetView(backbuffer, NULL, g_renderer.m_targetDescriptors[i]);
            g_renderer.mp_targetResources[i] = backbuffer;
        }
#endif

        OnResize();
    }

    bool Application::GetHasStarted() const
    {
        return m_hasStarted;
    }

    bool Application::GetShouldQuit() const
    {
        return m_shouldQuit;
    }


    const Application::Settings& Application::GetSettings() const
    {
        return m_currentSettings;
    }

    const Application::Settings& Application::GetCreationSettings() const
    {
        return m_creationSettings;
    }

    const Application::TimeStats& Application::GetTimeStats() const
    {
        return m_timeStats;
    }
}

