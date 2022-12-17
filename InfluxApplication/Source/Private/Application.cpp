#include "Application.h"

#include "Renderer.h"
#include "Engine.h"

namespace Influx::Application
{
    LRESULT Application::WindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
    {
        Application* currentAppInstance = sp_currentApplicationInstance;
        if (sp_currentApplicationInstance == nullptr) return ::DefWindowProc(hWnd, uMsg, wParam, lParam);

        if (currentAppInstance->GetHasUIRenderer())
        {
            if (currentAppInstance->mp_renderer != nullptr)
            {
                currentAppInstance->mp_renderer->WindowsProcedure(hWnd, uMsg, wParam, lParam);
            }
        }

        switch (uMsg)
        {
        case WM_CLOSE:

            break;

        case WM_SIZE:
        {
            Math::Rectu newRect = Platform::Window::GetClientRect<uint32_t>(hWnd);
            currentAppInstance->m_currentSettings.WindowDimensions.x = newRect.m_widthHeigth.x;
            currentAppInstance->m_currentSettings.WindowDimensions.y = newRect.m_widthHeigth.y;
        }
            break;

        case WM_DESTROY:
            
            break;
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

	Application::Application(const Settings& desc)
        : m_hasStarted{false}
        , m_shouldQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
	{
        
	}

    void Application::Run(int argc, char** argv)
    {
        ShutdownOtherApplication();

        Initialize();

        m_hasStarted = true;
        while (!m_shouldQuit)
        {
            if (GetHasWindow())
            {
                PollWindowEvents();
            }

            {
                Update();
                //std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }

            if (GetHasUIRenderer())
            {
                UIRender();
            }

            ++m_frame;
        }

        Cleanup();
    }

    void Application::Initialize()
    {
        if (m_isInitialized) return;

        sp_currentApplicationInstance = this;

        if (GetHasWindow())
        {
            CreateWindow();
        }

        if (GetHasUIRenderer())
        {
            mp_renderer = new ImGuiRendererDx12(m_currentSettings.WindowDimensions, m_windowHandle);
        }

        m_isInitialized = true;
    }

    void Application::Cleanup()
    {
        sp_currentApplicationInstance = nullptr;
    }

    void Application::PollWindowEvents()
    {
#if PLATFORM_WINDOWS
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0u, 0u, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                m_shouldQuit = true;
            }
        }
#endif
    }

    void Application::Update()
    {
        m_widget_logView.AddLog("[Info] Testing out here with number ... %i \n", m_frame);
        OnUpdate();
    }

    void Application::UIRender()
    {
        if (mp_renderer == nullptr)
        {
            return;
        }

        mp_renderer->RenderFrame([this]()
            {
                UIRender_ApplicationUI();
                OnUIRender(); 
            });
    }

    void Application::CreateWindow()
    {
#if PLATFORM_WINDOWS
        m_windowHandle = Platform::CreateWindow(
            static_cast<int>(m_currentSettings.WindowDimensions.x),
            static_cast<int>(m_currentSettings.WindowDimensions.y),
            Influx::ToWString(m_currentSettings.Name), Application::WindowsProcedure);
#endif
    }

    void Application::ShutdownOtherApplication()
    {
        if (sp_currentApplicationInstance != nullptr)
        {
            sp_currentApplicationInstance->SetQuit();
        }
    }

#pragma region UIRendering
    void Application::UIRender_ApplicationUI()
    {
        // [MAIN VIEWPORT]
        {
            ImGui::DockSpaceOverViewport();
        }

        UIRender_AppUI_FileMenu();
        UIRender_AppUI_AppInfo();
        UIRender_AppUI_EngineLog();

        ImGui::ShowDemoWindow();
    }

    void Application::UIRender_AppUI_FileMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {}
                if (ImGui::MenuItem("Open", "Ctrl+O")) {}

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Application::UIRender_AppUI_AppInfo()
    {
        ImGui::Begin("App Info");

        ImGui::Text("Settings");
        UIElement(m_currentSettings);

        ImGui::End();
    }

    void Application::UIRender_AppUI_EngineLog()
    {
        m_widget_logView.Draw("Log", nullptr);
    }

    void Application::UIElement(const Settings& settings)
    {
        ImGui::Text("[Type] %s", k_eApplicationTypeStrings[static_cast<uint8_t>(settings.Type)]);
        ImGui::Text("[Name] %s", settings.Name.data());
        ImGui::Text("[Window Size] %i, %i", settings.WindowDimensions.x, settings.WindowDimensions.y);
    }
#pragma endregion

    void Application::SetQuit()
    {
        m_shouldQuit = true;
    }

	Application::~Application()
	{

	}

    bool Application::GetHasStarted() const
    {
        return m_hasStarted;
    }

    bool Application::GetShouldQuit() const
    {
        return m_shouldQuit;
    }

    bool Application::GetHasWindow() const
    {
#if !PLATFORM_WINDOWS
        return false;
#endif

        return m_currentSettings.Type != EApplicationType::Minimal;
    }

    bool Application::GetHasUIRenderer() const
    {
        return m_currentSettings.Type == EApplicationType::ImGuiApp;
    }

    const Application::Settings& Application::GetSettings() const
    {
        return m_currentSettings;
    }

    const Application::Settings& Application::GetCreationSettings() const
    {
        return m_creationSettings;
    }
}

