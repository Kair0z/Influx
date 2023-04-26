#pragma once

#ifndef __CORE_WINDOWSPLATFORM_H_
#define __CORE_WINDOWSPLATFORM_H_

#include "Platform.h"

#include "Core/BasicTypes.h"
#include "Core/Cast.h"
#include "Core/Container/Containers.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// We don't like UNICODE steered macros! 
// We will always use the UNICODE A versions of these functions!
#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef MessageBox
#undef MessageBox
#endif

namespace Influx::Platform
{
	using WindowsProcedure = ::WNDPROC;

	namespace Internal
	{
		static List<WindowHandle>		gWindowHandles{};
		static List<WindowsProcedure>	gWindowsProcedureCallbacklist{};

		constexpr WindowEvent TranslateEvent(const uint32 value)
		{
			switch (value)
			{
			default:
			case WM_NULL:		return WindowEvent::Unknown;
			case WM_QUIT:		return WindowEvent::Quit;
			case WM_ACTIVATE:	return WindowEvent::Activate;
			}
		}

		inline LRESULT DefaultWindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
		{
			for (const WindowsProcedure& proc : gWindowsProcedureCallbacklist)
			{
				proc(hWnd, uMsg, wParam, lParam);
			}

			switch (uMsg)
			{
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}

			default:
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		template <typename _T>
		inline Math::Rect<_T> Cast(const ::RECT& rect)
		{
			return Math::Rect<_T>(
				sCast<_T>(rect.left),
				sCast<_T>(rect.bottom),
				sCast<_T>(rect.right - rect.left),
				sCast<_T>(rect.bottom - rect.top));
		}
	}

	// [ MEMORY ]
	inline void* Allocate(const uint64 size)
	{
		if (size == 0)
		{
			// https://stackoverflow.com/questions/2022335/whats-the-point-of-malloc0
			return malloc(1u);
		}
		else
		{
			return malloc(size);
		}
	}

	template <typename _T>
	inline _T* Allocate()
	{
		return static_cast<_T*>(Allocate(sizeof(_T)));
	}

	template <typename _T, typename ..._Args>
	inline _T* New(_Args&&... args)
	{
		return new _T(args...);
	}

	template <typename _T>
	inline void Free(_T* address)
	{
		std::free(address);
	}

	// [ THREADING ]
	namespace Thread
	{

	}

	// [ APPLICATION ]
	inline ProcessHandle GetCurrentProcess()
	{
		return ::GetCurrentProcess();
	}

	inline InstanceHandle GetCurrentInstance()
	{
		return ::GetModuleHandleW(NULL);
	}

	inline WindowHandle GetCurrentWindowHandle()
	{
		return ::GetActiveWindow();
	}

	inline bool IsWindowValid(WindowHandle handle)
	{
		return ::IsWindow((::HWND)handle);
	}

	inline void QuitCurrentInstance()
	{
		::PostQuitMessage(0);
	}

	inline void AddWindowsProcedureCallback(WindowsProcedure callback)
	{
		Internal::gWindowsProcedureCallbacklist.push_back(callback);
	}

	inline void RemoveWindowsProcedureCallback(WindowsProcedure callback)
	{
		Internal::gWindowsProcedureCallbacklist.remove(callback);
	}

	// [ WINDOW ]
	/* Returns false if a quit-event was polled! */
	inline bool PollWindowEvents(Vector<WindowEvent>& out_events, WindowHandle handle = GetCurrentWindowHandle())
	{
		// http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-4

		MSG msg;
		if (!IsWindowValid(handle))
		{
			return false;
		}
		
		bool hasQuitEvent = false;
		out_events.clear();
		out_events.reserve(16u); // how many could this really be xD?

		// process ALL windows event message
		while (::PeekMessage(&msg, (::HWND)handle, 0u, 0u, PM_REMOVE))
		{
			WindowEvent translatedEvent = Internal::TranslateEvent(msg.message);
			out_events.push_back(translatedEvent);

			if (translatedEvent == WindowEvent::Quit)
			{
				hasQuitEvent = true;
				break;
			}

			::TranslateMessage(&msg);

			// Dispatch to the WndProc
			::DispatchMessage(&msg);
		}

		return !hasQuitEvent;
	}

	/* Returns false if a quit-event was polled! */
	inline bool PollWindowEvents(WindowHandle handle = GetCurrentWindowHandle())
	{
		Vector<WindowEvent> out_events{};
		return PollWindowEvents(out_events, handle);
	}

	inline WindowHandle CreateWindow(const WindowSettings& settings, bool shouldOpen, WindowsProcedure windowsProcedureOverride)
	{
		::HINSTANCE instance = (::HINSTANCE)GetCurrentInstance();

		const WString nameWstring = ToWString(settings.Name);

		// [ REGISTER WINDOW CLASS ]
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			::UINT windowClassStyle{};
			::HBRUSH classBackgroundBrush = ::CreateSolidBrush(0x00000000);

			::WNDCLASSEXW windowClassExtended;
			windowClassExtended.cbSize			= sizeof(WNDCLASSEXW);
			windowClassExtended.style			= windowClassStyle;
			windowClassExtended.lpfnWndProc		= Internal::DefaultWindowsProcedure;
			windowClassExtended.cbClsExtra		= 0;
			windowClassExtended.cbWndExtra		= 0;
			windowClassExtended.hInstance		= instance;
			windowClassExtended.hIcon			= NULL;
			windowClassExtended.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
			windowClassExtended.hbrBackground	= classBackgroundBrush;
			windowClassExtended.lpszMenuName	= NULL;
			windowClassExtended.lpszClassName	= nameWstring.c_str();
			windowClassExtended.hIconSm			= ::LoadIcon(NULL, IDI_APPLICATION);

			if (!::RegisterClassExW(&windowClassExtended))
			{
				ErrorMessageBox("Fatal Error!", "Cannot Register Class", nullptr);
				return nullptr;
			}
		}

		// [ CREATE WINDOW CLASS ]
		::HWND newWindowHandle = NULL;
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
			::DWORD extendedWindowStyle{};
			::DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
			// Window-Frameless
			// style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION; 
			//style = style & (~WS_SIZEBOX);
			windowStyle &= ~WS_VISIBLE;

			// Middle of screen
			int xPos = (::GetSystemMetrics(SM_CXSCREEN) / 2) - (settings.Width / 2);
			int yPos = (::GetSystemMetrics(SM_CYSCREEN) / 2) - (settings.Heigth / 2);

			RECT clientArea;
			clientArea.left		= xPos;
			clientArea.top		= yPos;
			clientArea.right	= xPos + settings.Width;
			clientArea.bottom	= yPos + settings.Heigth;
			::AdjustWindowRect(&clientArea, windowStyle, FALSE);

			::HWND parentWindow = NULL;
			::HMENU parentMenu = NULL;

			newWindowHandle = ::CreateWindowExW(
				extendedWindowStyle,
				nameWstring.c_str(),
				nameWstring.c_str(),
				windowStyle,
				xPos, yPos, settings.Width, settings.Heigth,
				parentWindow, parentMenu, instance, NULL);

			if (newWindowHandle == NULL)
			{
				ErrorMessageBox("Fatal Error", "Failed Creating Window!", NULL);
				return nullptr;
			}
		}

		if (shouldOpen)
		{
			SetWindowVisibility(newWindowHandle, EWindowVisibility::ShowDefault);
		}
		else
		{
			SetWindowVisibility(newWindowHandle, EWindowVisibility::Minimize);
		}

		// Get Screen Refresh Rate
		{
			::DEVMODE lpDevMode;
			memset(&lpDevMode, 0, sizeof(::DEVMODE));
			lpDevMode.dmSize = sizeof(::DEVMODE);
			lpDevMode.dmDriverExtra = 0;

			if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
			{
				// float displayFrequency = static_cast<float>(lpDevMode.dmDisplayFrequency);
				///printf("Display Refresh Rate is %.2f Hz, setting fps_max to %i.\n\n", displayFrequency, (int)displayFrequency);
			}
		}

		// Initialize raw input
		{
			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = ((USHORT)0x01);
			Rid[0].usUsage = ((USHORT)0x02);
			Rid[0].dwFlags = /*RIDEV_INPUTSINK | RIDEV_DEVNOTIFY*/0;
			Rid[0].hwndTarget = newWindowHandle;
			if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
			{
				WarningMessageBox("Warning", "Failed RegisterRawInputDevices()!", nullptr);
			}
		}

		return newWindowHandle;
	}

	inline WindowHandle CreateWindow(const WindowSettings& settings, bool shouldOpen)
	{
		return CreateWindow(settings, shouldOpen, Internal::DefaultWindowsProcedure);
	}

	inline WindowSettings GetWindowSettings(const WindowHandle handle)
	{
		WindowSettings settings{};
		
		const Math::Recti& clientRect = GetClientWindowRect<int>(handle);
		settings.Width = clientRect.m_widthHeigth.x;
		settings.Heigth = clientRect.m_widthHeigth.y;
		// Todo... name?

		return settings;
	}

	inline void DestroyWindow(const WindowHandle handle)
	{
		::DestroyWindow((::HWND)handle);
	}

	inline bool SetWindowVisibility(const WindowHandle windowHandle, const EWindowVisibility command)
	{
		::HWND hwnd = (::HWND)windowHandle;

		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
		switch (command)
		{
		default:
			return false;
			
		case EWindowVisibility::Minimize:
			return ::CloseWindow(hwnd);
			break;

		case EWindowVisibility::ShowDefault:
			return ::ShowWindow(hwnd, SW_SHOWNORMAL);
			break;

		case EWindowVisibility::Maximize:
			return ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
			break;
		}
	}

	template <typename _T>
	inline Math::Rect<_T> GetFullWindowRect(const WindowHandle windowHandle)
	{
		::RECT res{};
		::GetWindowRect((::HWND)windowHandle, &res);

		return Cast<_T>(res);
	}

	template <typename _T>
	inline Math::Rect<_T> GetClientWindowRect(const WindowHandle windowHandle)
	{
		::RECT res{};
		::GetClientRect((::HWND)windowHandle, &res);

		return Internal::Cast<_T>(res);
	}

	template <typename _T>
	inline Math::Vector<_T, 2u> GetClientWindowDimensions(const WindowHandle windowHandle)
	{
		return GetClientWindowRect<_T>(windowHandle).GetDimensions();
	}

	inline bool IsWindowVisible(const WindowHandle windowHandle)
	{
		return ::IsWindowVisible((::HWND)windowHandle);
	}

	// [ MISC ]
	template <EMessageBoxType _T>
	inline void MessageBox(const String& caption, const String& message, const WindowHandle windowHandle)
	{
		uint8 type = 0u;
		switch (_T)
		{
		default:
		case EMessageBoxType::Info:
			type = MB_ICONINFORMATION | MB_OK;
			break;

		case EMessageBoxType::Warning:
			type = MB_ICONWARNING | MB_OK;
			break;

		case EMessageBoxType::Error:
			type = MB_ICONEXCLAMATION | MB_OK;
			break;
		}
		
		::MessageBoxA((::HWND)windowHandle, message.c_str(), caption.c_str(), type);
	}

	template <EConsoleColour _C>
	inline void SetConsoleColourAttribute()
	{
		constexpr int value = static_cast<int>(_C);

		HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::SetConsoleTextAttribute(hConsole, value);
	}
}

#endif // PLATFORM_WINDOWS
