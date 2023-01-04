#include "Application.h"

#include "InfluxEngine/Engine.h"
#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "Rendering/ApplicationRenderer.h"

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
        , mp_rhiDevice{nullptr}
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
            CreateGraphics();
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

        static Vector<Platform::Window::Event> events{};
        m_shouldQuit = Platform::PollWindowEvents(events);
    }

    void Application::Update()
    {
        mp_engine->Tick();
    }

    void Application::SceneRender()
    {
        if (!GetShouldRenderScene())
        {
            return;
        }


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

        Platform::Window::Settings windowSettings{};
        windowSettings.Width    = GetSettings().WindowDimensions.x;
        windowSettings.Heigth   = GetSettings().WindowDimensions.y;
        windowSettings.Name     = GetSettings().Name;

        m_windowHandle = Platform::Window::Create(windowSettings);
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

    void Application::CreateGraphics()
    {
        if (GetHasCreatedGraphics())
        {
            return;
        }

#if FLX_APP_RENDERER_D3D12
        mp_rhiDevice = Platform::New<Graphics::D3D12Device>();
#endif
    }

    void Application::CreateRenderer()
    {
        if (!GetShouldHaveWindow() || !GetHasCreatedGraphics() || !GetHasCreatedWindow())
        {
            return;
        }

        mp_appRenderer = Platform::New<ApplicationRenderer>(mp_rhiDevice, m_windowHandle);
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

    bool Application::GetHasCreatedGraphics() const
    {
        return mp_rhiDevice != nullptr;
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

