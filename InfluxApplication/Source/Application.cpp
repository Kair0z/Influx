#include "Application.h"

#include "Core/Platform/WindowsPlatform.h"

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
        WindowsPlatform::CreateWindow(
            static_cast<int>(desc.InitWindowSize.x),
            static_cast<int>(desc.InitWindowSize.y),
            Influx::ToWString(desc.Name), WndProc);
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

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (m_updateClb != nullptr)
            {
                m_updateClb(*this);
            }

            if (m_uiRenderClb != nullptr)
            {
                m_uiRenderClb(*this);
            }
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
}

