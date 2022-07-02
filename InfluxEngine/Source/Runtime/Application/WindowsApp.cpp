#include "pch.h"
#include "Runtime/Application/WindowsApp.h"
#include "Runtime/Window/Window.h"

#include "Runtime/Events/EventManager.h"
#include "Runtime/Engine/EngineEvents.h"
#include "Runtime/Application/WindowEvents.h"
#include "Platform/WindowsPlatform.h"

namespace Influx
{
	Vector<WindowsApp::WindowProcHandler> WindowsApp::StatExternalWindowProcHandlers = {};

	Ptr<WindowsApp> WindowsApp::Create(const HINSTANCE instance, const HICON icon)
	{
		Ptr<WindowsApp> newApp = new WindowsApp();

		// TODO: Should we make a new window automatically?
		Window::CreateDesc desc{};
		desc.Height = 720;
		desc.Width = 1280;
		desc.name = "App";
		newApp->mpWindow = newApp->MakeWindow(desc);
		newApp->mpWindow->Show();

		return newApp;
	}

	Ptr<Window> WindowsApp::GetWindow() const
	{
		return mpWindow;
	}

	void WindowsApp::PollEvents()
	{
		MSG msg{};
		while (::PeekMessage(&msg, mpWindow->GetWindowsHandle(), 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	void WindowsApp::AddWindowEventProcHandler(WindowProcHandler handler)
	{
		StatExternalWindowProcHandlers.push_back(handler);
	}

	Ptr<Window> WindowsApp::MakeWindow(const Window::CreateDesc& desc, const bool show)
	{
		return Window::Create(desc, NULL, WindowsApp::WndProc);
	}

	LRESULT WindowsApp::WndProc(HWND hwnd, UINT mssg, WPARAM wParam, LPARAM lParam)
	{
		LPARAM abc = lParam;
		LRESULT ret = DefWindowProcW(hwnd, mssg, wParam, lParam);

		/* External Procedure Hooks... */
		for (WindowProcHandler f : StatExternalWindowProcHandlers)
		{
			f(hwnd, mssg, wParam, lParam);
		}

		if (!EventManagerLocator::Get()) return ret;

		switch (mssg)
		{
		case WM_DESTROY:
			EventManagerLocator::Get()->PingChannelImmediate<EventCategory::Engine, EngineQuitEvent>();
			return 0;

		case WM_CLOSE:
			// EventmanagerLocator::Get()->PingChannelImmediate<EventCategory::Engine, EngineQuitEvent>();
			return 0;

		case WM_SYSKEYDOWN:
			// gpEngine->OnKeyDown(wParam);
			break;

		case WM_SYSKEYUP:
			// gpEngine->OnKeyUp(wParam);
			break;

		case WM_CHAR:
			// gpEngine->OnKeyChar(wParam);
			break;

		case WM_LBUTTONDOWN:
			// gpEngine->OnMouseLeft(true);
			break;

		case WM_MBUTTONDOWN:
			// gpEngine->OnMouseMiddle(true);
			break;

		case WM_RBUTTONDOWN:
			// gpEngine->OnMouseRight(true);
			break;

		case WM_LBUTTONUP:
			// gpEngine->OnMouseLeft(false);
			break;

		case WM_MBUTTONUP:
			// gpEngine->OnMouseMiddle(false);
			break;

		case WM_RBUTTONUP:
			// gpEngine->OnMouseRight(false);
			break;

		case WM_XBUTTONDOWN:
			if (GET_XBUTTON_WPARAM(wParam) == VK_XBUTTON1)
			{
				// gpEngine->OnMouse4(true);
				break;
			}
			if (GET_XBUTTON_WPARAM(wParam) == VK_XBUTTON2)
			{
				// gpEngine->OnMouse5(true);
				break;
			}
			break;

		case WM_XBUTTONUP:
			if (GET_XBUTTON_WPARAM(wParam) == VK_XBUTTON1)
			{
				// gpEngine->OnMouse4(false);
				break;
			}
			if (GET_XBUTTON_WPARAM(wParam) == VK_XBUTTON2)
			{
				// gpEngine->OnMouse5(false);
				break;
			}
			break;

			// Raw input handling (For mouse movement):
		case WM_INPUT:
		{
			RAWINPUT raw;
			UINT dwSize = sizeof(raw);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));
			if (raw.header.dwType == RIM_TYPEMOUSE)
			{
				// TODO
				// raw.data.mouse.lLastX;
				// raw.data.mouse.lLastY;
				// raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE;
				// raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP;
			}
		}
		break;

		// Mouse Wheels:
		case WM_MOUSEWHEEL:
			// gpEngine->OnMouseWheelVertical(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_MOUSEHWHEEL:
			// gpEngine->OnMouseWheelHorizontal(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

			// Min/Max:
		case WM_SYSCOMMAND:
			switch (wParam)
			{
			case SC_MINIMIZE:
				// gpEngine->OnMinimized();
				break;

			case SC_MAXIMIZE:
				// gpEngine->OnMaximized();
				break;

			case SC_RESTORE:
				// gpEngine->OnRestored();
				break;
			}
			break;

		case WM_EXITSIZEMOVE:
		{
			WindowResizeEvent windowEvent{};
			windowEvent.NewHeight = (int)WindowsPlatform::GetWindowClientRect().WH.x;
			windowEvent.NewWidth = (int)WindowsPlatform::GetWindowClientRect().WH.y;
			if (windowEvent.NewHeight == 0 && windowEvent.NewWidth == 0) break;

			EventManagerLocator::Get()->PingChannelImmediate<EventCategory::Window, WindowResizeEvent>(windowEvent);
		}
		break;
		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			break;
		case WM_GETMINMAXINFO:
			break;
		}

		return ret;
	}
}
