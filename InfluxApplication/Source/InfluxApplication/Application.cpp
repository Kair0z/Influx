#include "app_pch.h"

#include "InfluxApplication/Application.h"
#include "InfluxEngine/Engine.h"
#include "InfluxRenderer/RootRenderer.h"

namespace Influx::Application
{
	Application::Application(const Settings& desc)
        : m_hasStarted{false}
        , m_recievedQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
        , m_deltaTime{}
        , m_frame{}
        , m_isInitialized{false}
        , m_time{}
        , m_hasCleanedUp{false}
        , mp_engine{nullptr}
        , mp_appRenderer{}
        , m_windowHandle{nullptr}
        , m_processHandle{nullptr}
        , m_appInstanceHandle{nullptr}
        , m_hasCreatedWindow{false}
	{
        
	}

    Application::~Application()
    {
        Cleanup();
    }

    void Application::Run(int argc, char** argv)
    {
        Initialize();

        Start();

        while (!GetHasRecievedQuit())
        {
            if (GetShouldHaveWindow())
            {
                PollWindowEvents();
            }

            if (GetShouldHaveUpdate())
            {
                Update();
            }

            if (GetShouldRenderScene())
            {
                SceneRender();
            }

            if (GetShouldHaveImgui())
            {
                ImguiRender();
            }

            ++m_frame;
        }

        Cleanup();
    }

    void Application::Initialize()
    {
        if (m_isInitialized) return;

        m_processHandle     = Platform::GetCurrentProcess();
        m_appInstanceHandle = Platform::GetCurrentInstance();

        CreateEngine();

        if (GetShouldHaveWindow())
        {
            CreateWindow();
            CreateRenderer();
        }

        if (GetShouldHaveImgui())
        {
            // Todo...
        }

        m_isInitialized = true;
    }

    void Application::Cleanup()
    {
        if (GetHasCreatedWindow())
        {
            Platform::DestroyWindow(m_windowHandle);
        }

        if (GetHasCreatedRenderer())
        {
            Renderer::RootRenderer::Destroy(mp_appRenderer);
        }

        if (GetHasCreatedEngine())
        {
            Engine::Destroy(mp_engine);
        }

        m_hasCleanedUp = true;
    }

    void Application::Start()
    {
        if (m_hasStarted)
        {
            return;
        }

        m_hasStarted = true;
    }

    void Application::PollWindowEvents()
    {
        const bool shouldHaveWindow = GetShouldHaveWindow();
        const bool hasCreatedWindow = GetHasCreatedWindow();

        if (!shouldHaveWindow || !hasCreatedWindow)
        {
            return;
        }

        const bool hasRecievedQuit = GetHasRecievedQuit();

        if (!hasRecievedQuit)
        {
            bool hasQuit = !Platform::PollWindowEvents(m_windowHandle);
            m_recievedQuit = hasQuit;
        }
    }

    void Application::Update()
    {
        const bool hasCreatedEngine = GetHasCreatedEngine();

        if (!hasCreatedEngine)
        {
            return;
        }

        mp_engine->Tick();
    }

    void Application::SceneRender()
    {
        const bool shouldRenderScene = GetShouldRenderScene();
        const bool hasCreatedWindow = GetHasCreatedWindow();
        const bool hasCreatedRenderer = GetHasCreatedRenderer();

        if (!shouldRenderScene || !hasCreatedWindow || !hasCreatedRenderer)
        {
            return;
        }

        mp_appRenderer->Render();
        mp_appRenderer->Present(true);
    }

    void Application::ImguiRender()
    {
        if (!GetShouldHaveImgui())
        {
            return;
        }
    }

    void Application::CreateWindow()
    {
        const bool shouldHaveWindow = GetSettings().HasWindow;
        
        if (!shouldHaveWindow)
        {
            return;
        }

        Platform::WindowSettings windowSettings{};
        windowSettings.Width    = GetSettings().WindowDimensions.x;
        windowSettings.Heigth   = GetSettings().WindowDimensions.y;
        windowSettings.Name     = GetSettings().Name;

        const bool shouldOpen = true;
        m_windowHandle = Platform::CreateWindow(windowSettings, shouldOpen);

        m_hasCreatedWindow = true;
    }

    void Application::CreateEngine()
    {
        if (GetHasCreatedEngine())
        {
            return;
        }

        Engine::ConstructArgs constrArgs{};
        
        mp_engine = Engine::Create(constrArgs);
    }

    void Application::CreateRenderer()
    {
        if (!GetShouldHaveWindow() || !GetHasCreatedWindow())
        {
            return;
        }

        mp_appRenderer = Renderer::RootRenderer::Create(Graphics::EGraphicsAPI::D3D12, m_windowHandle);
    }

    void Application::SignalQuit()
    {
        m_recievedQuit = true;
    }


    bool Application::GetHasStarted() const
    {
        return m_hasStarted;
    }

    bool Application::GetHasRecievedQuit() const
    {
        return m_recievedQuit;
    }

    bool Application::GetShouldHaveWindow() const
    {
        return GetSettings().HasWindow;
    }

    bool Application::GetShouldHaveImgui() const
    {
        return GetSettings().HasUI && GetShouldHaveWindow();
    }

    bool Application::GetShouldRenderScene() const
    {
        return GetSettings().HasSceneRender && GetShouldHaveWindow();
    }

    bool Application::GetShouldHaveUpdate() const
    {
        return GetSettings().HasUpdate;
    }

    bool Application::GetHasCleanedUp() const
    {
        return m_hasCleanedUp;
    }

    bool Application::GetHasCreatedWindow() const
    {
        return m_hasCreatedWindow && m_windowHandle != nullptr;
    }

    bool Application::GetHasCreatedRenderer() const
    {
        return mp_appRenderer != nullptr;
    }

    bool Application::GetHasCreatedEngine() const
    {
        return mp_engine != nullptr;
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

