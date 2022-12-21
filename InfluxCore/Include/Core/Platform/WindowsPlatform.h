#pragma once

#ifndef _CORE_PLATFORM_WINDOWS_H_
#define _CORE_PLATFORM_WINDOWS_H_
#if PLATFORM_WINDOWS

#include "Core/Cast.h"
#include "Core/Container/Vector.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace Influx::Platform
{
	// Memory 
	template <typename _T>
	inline _T* Allocate()
	{
		return static_cast<_T*>(std::malloc(sizeof(_T)));
	}

	template <typename _T>
	inline void Free(_T* address)
	{
		std::free(address);
	}

	// Process & Window
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

	inline void QuitCurrentInstance()
	{
		::PostQuitMessage(0);
	}

	namespace
	{
		constexpr Window::Event TranslateEvent(uint8 value)
		{
			switch (value)
			{
			default:
			case WM_NULL: return Window::Event::Unknown;
			case WM_QUIT: return Window::Event::Quit;
			case WM_ACTIVATE: return Window::Event::Activate;
			}
		}
	}

	/// <summary>
	/// Returns 'HasQuitMessage'
	/// </summary>
	/// <param name="handle"></param>
	/// <param name=""></param>
	inline bool PollWindowEvents(Vector<Window::Event>& out_events, WindowHandle handle = GetCurrentWindowHandle())
	{
		// http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-4

		MSG msg;
		
		bool hasQuitEvent = false;
		out_events.clear();

		// process ALL windows event message
		while (::PeekMessage(&msg, (::HWND)handle, 0, 0, PM_REMOVE))
		{
			Window::Event translatedEvent = TranslateEvent(msg.message);
			out_events.push_back(translatedEvent);

			if (translatedEvent == Window::Event::Quit)
			{
				hasQuitEvent = true;
			}

			::TranslateMessage(&msg);

			// Dispatch to the WndProc
			::DispatchMessage(&msg);
		}

		return hasQuitEvent;
	}

	namespace Window
	{
		namespace
		{
			using WindowsProcedure = ::WNDPROC;
			inline LRESULT DefaultWindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
			{
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
		}

		inline void ErrorBox(const String& errorString, const String& errorCaption, WindowHandle windowsProcedure)
		{
			::MessageBox((::HWND)windowsProcedure, ToWString(errorString).c_str(), ToWString(errorCaption).c_str(),
				MB_ICONEXCLAMATION | MB_OK);
		}

		inline WindowHandle Create(const Settings& settings, WindowsProcedure windowsProcedureOverride)
		{
			// prepare window class
			::HINSTANCE instance = (::HINSTANCE)GetCurrentInstance();
			auto nameWString = ToWString(settings.Name);

			// REGISTER WINDOW CLASS
			{
				// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
				::UINT windowClassStyle{};
				::HBRUSH classBackgroundBrush = ::CreateSolidBrush(0x00000000);

				WNDCLASSEX windowClassExtended;
				windowClassExtended.cbSize			= sizeof(WNDCLASSEX);
				windowClassExtended.style			= windowClassStyle;
				windowClassExtended.lpfnWndProc		= DefaultWindowsProcedure;
				windowClassExtended.cbClsExtra		= 0;
				windowClassExtended.cbWndExtra		= 0;
				windowClassExtended.hInstance		= instance;
				windowClassExtended.hIcon			= NULL;
				windowClassExtended.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
				windowClassExtended.hbrBackground	= classBackgroundBrush;
				windowClassExtended.lpszMenuName	= NULL;
				windowClassExtended.lpszClassName	= nameWString.c_str();
				windowClassExtended.hIconSm			= ::LoadIcon(NULL, IDI_APPLICATION);

				if (!::RegisterClassEx(&windowClassExtended))
				{
					ErrorBox("Cannot Register Class", "Fatal Error!", NULL);
					return nullptr;
				}
			}
			
			// CREATE WINDOW CLASS
			::HWND newWindow = NULL;
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

				newWindow = ::CreateWindowEx(
					extendedWindowStyle,
					nameWString.c_str(),
					nameWString.c_str(),
					windowStyle,
					xPos, yPos, settings.Width, settings.Heigth,
					parentWindow, parentMenu, instance, NULL);

				if (newWindow == NULL)
				{
					ErrorBox("Failed Creating Window!", "Fatal Error", NULL);
					return nullptr;
				}
			}
			ShowWindow(newWindow, SW_RESTORE);

			// Get Screen Refresh Rate
			{
				DEVMODE lpDevMode;
				memset(&lpDevMode, 0, sizeof(DEVMODE));
				lpDevMode.dmSize = sizeof(DEVMODE);
				lpDevMode.dmDriverExtra = 0;

				if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
				{
					float displayFrequency = static_cast<float>(lpDevMode.dmDisplayFrequency);
					///printf("Display Refresh Rate is %.2f Hz, setting fps_max to %i.\n\n", displayFrequency, (int)displayFrequency);
				}
			}

			// Initialize raw input
			{
				RAWINPUTDEVICE Rid[1];
				Rid[0].usUsagePage = ((USHORT)0x01);
				Rid[0].usUsage = ((USHORT)0x02);
				Rid[0].dwFlags = /*RIDEV_INPUTSINK | RIDEV_DEVNOTIFY*/0;
				Rid[0].hwndTarget = newWindow;
				if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
				{
					MessageBox(NULL, L"Couldn't RegisterRawInputDevices()!", L"Warning", MB_ICONEXCLAMATION | MB_OK);
				}
			}

			return newWindow;
		}

		inline WindowHandle Create(const Settings& settings)
		{
			return Create(settings, DefaultWindowsProcedure);
		}

		inline void Destroy(WindowHandle handle)
		{
			::DestroyWindow((::HWND)handle);
		}

		namespace // Cast a ::RECT -> Math::Rect<_T>
		{
			template <typename _T>
			inline Math::Rect<_T> Cast(const ::RECT& rect)
			{
				return Math::Rect<_T>(
					StaticCast<_T>(rect.left),
					StaticCast<_T>(rect.bottom),
					StaticCast<_T>(rect.right - rect.left),
					StaticCast<_T>(rect.bottom - rect.top));
			}
		}

		template <typename _T>
		inline Math::Rect<_T> GetFullRect(WindowHandle handle)
		{
			::RECT res{};
			::GetWindowRect((::HWND)handle, &res);

			return Cast<_T>(res);
		}

		template <typename _T>
		inline Math::Rect<_T> GetClientRect(WindowHandle handle)
		{
			::RECT res{};
			::GetClientRect((::HWND)handle, &res);

			return Cast<_T>(res);
		}

		inline bool IsVisible(WindowHandle handle)
		{
			return ::IsWindowVisible((::HWND)handle);
		}
	}
	
	namespace Console
	{
		template <EColourAttribute _A>
		inline void SetColourAttribute()
		{
			constexpr int value = static_cast<int>(_A);

			HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
			::SetConsoleTextAttribute(hConsole, value);
		}
	}
}

#endif // PLATFORM_WINDOWS
#endif