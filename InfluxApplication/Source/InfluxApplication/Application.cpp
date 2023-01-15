#include "app_pch.h"

#include "InfluxApplication/Application.h"
#include "InfluxEngine/Engine.h"

namespace Influx::Application
{
	Application::Application(const Settings& desc)
        : m_hasStarted{false}
        , m_shouldQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
        , m_deltaTime{}
        , m_frame{}
        , m_isInitialized{false}
        , m_time{}
        , m_hasCleanedUp{false}
        , mp_engine{nullptr}
        , m_appRenderer{}
	{
        
	}

    void Application::Run(int argc, char** argv)
    {
        Initialize();

        Start();

        while (!GetShouldQuit())
        {
            if (GetShouldHaveWindow())
            {
                PollWindowEvents();
            }

            if (GetHasUpdate())
            {
                Update();
            }

            if (GetShouldRenderScene())
            {
                SceneRender();
            }

            if (GetShouldHaveUI())
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

        m_processHandle = Platform::GetCurrentProcess();
        m_appInstanceHandle = Platform::GetCurrentInstance();

        CreateEngine();

        if (GetShouldHaveWindow())
        {
            CreateWindow();
            CreateRenderer();
        }

        if (GetShouldHaveUI())
        {
            // Todo...
        }

        m_isInitialized = true;
    }

    void Application::Start()
    {
        if (m_hasStarted)
        {
            return;
        }

        m_hasStarted = true;
    }

    void Application::Cleanup()
    {
        Engine::Destroy(mp_engine);

    }

    void Application::PollWindowEvents()
    {
        if (!GetShouldHaveWindow())
        {
            return;
        }

        if (!GetHasCreatedWindow())
        {
            return;
        }

        m_shouldQuit = Platform::PollWindowEvents(m_windowHandle);
    }

    void Application::Update()
    {
        if (!GetHasCreatedEngine())
        {
            return;
        }

        mp_engine->Tick();
    }

    void Application::SceneRender()
    {
        if (!GetShouldRenderScene())
        {
            return;
        }

        if (!GetHasCreatedWindow())
        {
            return;
        }

        m_appRenderer.Render();
        m_appRenderer.Present(true);
    }

    void Application::UIRender()
    {
        if (!GetShouldHaveUI())
        {
            return;
        }
    }

    void Application::CreateWindow()
    {
        if (!GetSettings().HasWindow)
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

        m_appRenderer.Initialize(Graphics::EGraphicsAPI::D3D12);
        m_appRenderer.AttachToWindow(m_windowHandle);
    }

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

    bool Application::GetShouldHaveWindow() const
    {
        return GetSettings().HasWindow;
    }

    bool Application::GetShouldHaveUI() const
    {
        return GetSettings().HasUI && GetShouldHaveWindow();
    }

    bool Application::GetShouldRenderScene() const
    {
        return GetSettings().HasSceneRender && GetShouldHaveWindow();
    }

    bool Application::GetHasUpdate() const
    {
        return GetSettings().HasUpdate;
    }

    bool Application::GetHasCleanedUp() const
    {
        return m_hasCleanedUp;
    }

    bool Application::GetHasCreatedWindow() const
    {
        return m_hasCreatedWindow;
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

