#include "Application.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Renderer.h"

inline LRESULT CALLBACK WndProc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    switch (uMsg)
    {
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

namespace Influx
{
	Application::Application(int argc, char** argv, const ApplicationDescription& desc)
        : m_hasStarted{false}
        , m_shouldQuit{false}
        , m_initDescription{desc}
	{
#if PLATFORM_WINDOWS
        m_windowHandle = WindowsPlatform::CreateWindow(
            static_cast<int>(desc.InitWindowSize.x),
            static_cast<int>(desc.InitWindowSize.y),
            Influx::ToWString(desc.Name), WndProc);
#endif
	}

    void Application::SetUIRenderCallback(OnUIRenderCallback newClb)
    {
        m_uiRenderClb = newClb;
    }

    void Application::SetUpdateCallback(OnUpdateCallback newClb)
    {
        m_updateClb = newClb;
    }

    void Application::Run()
    {
        m_hasStarted = true;

        if (!AreRequiredCallbacksRegistered())
        {
            Quit();
        }

        InitializeRenderer();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (m_updateClb != nullptr)
            {
                m_updateClb(*this);
            }

            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (m_uiRenderClb != nullptr)
            {
                m_uiRenderClb(*this);
            }

            ImGui::Render();
        }
    }

    void Application::Quit()
    {
        m_shouldQuit = true;
    }

	Application::~Application()
	{

	}

    bool Application::AreRequiredCallbacksRegistered() const
    {
        if (m_initDescription.Type == EApplicationType::Default)
        {
            // For this type, we require both callbacks to be registered!
            if (m_uiRenderClb != nullptr && m_updateClb != nullptr) return true;
        }
    }

    void Application::InitializeRenderer()
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
        ImGui_ImplDX12_Init(g_renderer.mp_dxDevice2, k_numFramesInFlight,
            DXGI_FORMAT_R8G8B8A8_UNORM, g_renderer.mp_dxSrvDescHeap,
            g_renderer.mp_dxSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            g_renderer.mp_dxSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

#if FLX_APP_RENDERER_DEBUG
        D3D12::EnableDxDebugLayer();
#endif

        IDXGIFactory4* factory4 = D3D12::CreateDxgiFactory4();
        IDXGIAdapter4* adapter4 = D3D12::GetDxgiAdapter4(factory4, k_useWarp);
        g_renderer.mp_dxDevice2 = D3D12::CreateDxDevice2(adapter4);

        constexpr D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
        D3D12::CreateDxCommandQueue(g_renderer.mp_dxDevice2, cmdListType);

        g_renderer.mp_dxRtvDescHeap = D3D12::CreateDxDescriptorHeap(g_renderer.mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, k_numFramesInFlight);
        g_renderer.mp_dxSrvDescHeap = D3D12::CreateDxDescriptorHeap(g_renderer.mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1u);

        for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
            g_renderer.m_frameContexts[i].CommandAllocator = D3D12::CreateDxCommandAllocator(g_renderer.mp_dxDevice2, cmdListType);

        g_renderer.mp_dxCommandList = D3D12::CreateDxCommandList(g_renderer.mp_dxDevice2,
            g_renderer.m_frameContexts[0].CommandAllocator, cmdListType);

        g_renderer.m_fence = D3D12::CreateDxFence(g_renderer.mp_dxDevice2);
        g_renderer.m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        g_renderer.mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(factory4, m_windowHandle, g_renderer.mp_dxCommandQueue,
            m_initDescription.InitWindowSize.x, m_initDescription.InitWindowSize.y, k_numFramesInFlight);
#endif

#endif

        // Call this to resize the rendertargets
        OnWindowResize();
    }

    void Application::OnWindowResize()
    {
#if FLX_APP_RENDERER_D3D12
        for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
        {
            ID3D12Resource* backbuffer = nullptr;
            g_renderer.mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer));
            g_renderer.mp_targetResources[i] = backbuffer;
            g_renderer.mp_dxDevice2->CreateRenderTargetView(backbuffer, NULL, g_renderer.m_targetDescriptors[i]);
        }
#endif
    }
}

