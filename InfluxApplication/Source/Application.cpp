#include "Application.h"

#include "Renderer.h"
#include "Engine.h"

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
	{
        
	}

    void Application::Run(int argc, char** argv)
    {
        Initialize();

        Start();

        while (!GetShouldQuit())
        {
            if (GetHasWindow())
            {
                PollWindowEvents();
            }

            if (GetHasUpdate())
            {
                Update();
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

        m_processHandle = Platform::GetCurrentProcess();
        m_appInstanceHandle = Platform::GetCurrentInstance();

        if (GetHasWindow())
        {
            CreateWindow();
        }

        if (GetHasUIRenderer())
        {

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

    }

    void Application::PollWindowEvents()
    {
        if (!GetHasWindow())
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
        //std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    void Application::UIRender()
    {
        
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
        return GetSettings().HasWindow;
    }

    bool Application::GetHasUIRenderer() const
    {
        return GetSettings().HasUI;
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

    const Application::Settings& Application::GetSettings() const
    {
        return m_currentSettings;
    }

    const Application::Settings& Application::GetCreationSettings() const
    {
        return m_creationSettings;
    }
}

