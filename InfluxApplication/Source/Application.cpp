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
	{
        WindowsPlatform::CreateWindow(
            static_cast<int>(desc.InitWindowSize.x),
            static_cast<int>(desc.InitWindowSize.y),
            Influx::ToWString(desc.Name), WndProc);
	}

    void Application::Run(OnUIRenderCallback onUIRender)
    {
        m_hasStarted = true;

        while (!m_shouldQuit)
        {
            onUIRender();
        }
    }

    void Application::Quit()
    {
        m_shouldQuit = true;
    }

	Application::~Application()
	{

	}
}

