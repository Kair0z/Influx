#include "Application.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Renderer.h"

namespace Influx::Application
{
    inline Application* Application::sp_currentApplicationInstance = nullptr;

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

        case WM_DESTROY:
            WindowsPlatform::Quit(0);
            break;
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

	Application::Application(int argc, char** argv, const Settings& desc)
        : m_hasStarted{false}
        , m_shouldQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
	{
        sp_currentApplicationInstance = this;
	}

    void Application::Run()
    {
        Initialize();

        m_hasStarted = true;
        while (!m_shouldQuit)
        {
            if (GetHasWindow())
            {
                // [POLL Windows Messages]
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
    }

    void Application::Initialize()
    {
        if (m_isInitialized) return;

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

    void Application::Update()
    {
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
        m_windowHandle = WindowsPlatform::CreateWindow(
            static_cast<int>(m_currentSettings.WindowDimensions.x),
            static_cast<int>(m_currentSettings.WindowDimensions.y),
            Influx::ToWString(m_currentSettings.Name), Application::WindowsProcedure);
#endif
    }

    void Application::UIRender_ApplicationUI()
    {
        // [MAIN VIEWPORT]
        {
            ImGui::DockSpaceOverViewport();
        }


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

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Log");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);

            
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::ShowDemoWindow();
    }

    void Application::Quit()
    {
        m_shouldQuit = true;
    }

    void Application::OnUIRender()
    {
        
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

    const Application::TimeStats& Application::GetTimeStats() const
    {
        return m_timeStats;
    }
}

