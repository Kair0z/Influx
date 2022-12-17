#pragma once

#ifndef _CORE_PLATFORM_WINDOWS_H_
#define _CORE_PLATFORM_WINDOWS_H_
#if 1

#include "Platform.h"
#include "Core/Cast.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace Influx::Platform
{
	// Memory 
	template <typename _T>
	_T* Allocate(const uint64 numBytes)
	{
		return static_cast<_T*>(std::malloc(numBytes));
	}

	template <typename _T>
	void Free(_T* address)
	{
		std::free(address);
	}

	// Process & Window
	ProcessHandle GetCurrentProcess()
	{
		return ::GetCurrentProcess();
	}

	InstanceHandle GetCurrentInstance()
	{
		return ::GetModuleHandleW(NULL);
	}

	WindowHandle GetCurrentWindowHandle()
	{
		return ::GetActiveWindow();
	}

	void QuitCurrentInstance()
	{
		::PostQuitMessage(0);
	}

	namespace Window
	{
		namespace
		{
			using WindowsProcedure = ::WNDPROC;
			LRESULT DefaultWindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
			{
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}

		WindowHandle Create(const Settings& settings, WindowsProcedure windowsProcedureOverride)
		{
			// prepare window class
			WNDCLASSEXW wc;
			::HINSTANCE i = (::HINSTANCE)GetCurrentInstance();

			auto nameWString = ToWString(settings.Name);

			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = 0;
			wc.lpfnWndProc = DefaultWindowsProcedure;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = i;
			wc.hIcon = NULL;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)CreateSolidBrush(0x00000000);
			wc.lpszMenuName = NULL;
			wc.lpszClassName = nameWString.c_str();
			wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

			// register window class
			if (!RegisterClassExW(&wc))
			{
				MessageBox(NULL, L"Couldn't RegisterClassEx()!", L"Fatal Error", MB_ICONEXCLAMATION | MB_OK);
				return {};
			}

			LONG_PTR style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
			//style = style & (~WS_SIZEBOX);

#ifdef WINDOW_FRAMELESS
			style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION;
#endif
			LONG_PTR exStyle = WS_EX_WINDOWEDGE;

			style &= ~WS_VISIBLE;

			int xPos = (GetSystemMetrics(SM_CXSCREEN) / 2) - (settings.w / 2);
			int yPos = (GetSystemMetrics(SM_CYSCREEN) / 2) - (settings.h / 2);
			int width = settings.w;
			int height = settings.h;

			RECT clientArea;
			clientArea.left = xPos;
			clientArea.top = yPos;
			clientArea.right = xPos + width;
			clientArea.bottom = yPos + height;
			AdjustWindowRect(&clientArea, (DWORD)style, FALSE);

			xPos = clientArea.left;
			yPos = clientArea.top;
			width = clientArea.right - clientArea.left;
			height = clientArea.bottom - clientArea.top;

			::HWND hwnd = CreateWindowExW((DWORD)exStyle,
				nameWString.c_str(),
				nameWString.c_str(),
				(DWORD)style,
				xPos, yPos, width, height,
				NULL, NULL, i, NULL);

			if (hwnd == NULL)
			{
				MessageBox(NULL, L"Error: Failed Creating Window!", L"Fatal Error", MB_ICONEXCLAMATION | MB_OK);
				return {};
			}

			// Get Screen Refresh Rate
			{
				DEVMODE lpDevMode;
				memset(&lpDevMode, 0, sizeof(DEVMODE));
				lpDevMode.dmSize = sizeof(DEVMODE);
				lpDevMode.dmDriverExtra = 0;

				if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
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
				Rid[0].hwndTarget = hwnd;
				if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
				{
					MessageBox(NULL, L"Couldn't RegisterRawInputDevices()!", L"Warning", MB_ICONEXCLAMATION | MB_OK);
				}
			}

			// Show the window
			ShowWindow(hwnd, SW_RESTORE);
#ifdef WINDOW_MAXIMIZED
			ShowWindow(hwnd, SW_MAXIMIZE);
#endif

			return hwnd;
		}

		WindowHandle Create(const Settings& settings)
		{
			return Create(settings, DefaultWindowsProcedure);
		}

		void Destroy(WindowHandle handle)
		{
			::DestroyWindow((::HWND)handle);
		}

		namespace // Cast a ::RECT -> Math::Rect<_T>
		{
			template <typename _T>
			Math::Rect<_T> Cast(const ::RECT& rect)
			{
				return Math::Rect<_T>(
					StaticCast<_T>(rect.left),
					StaticCast<_T>(rect.bottom),
					StaticCast<_T>(rect.right - rect.left),
					StaticCast<_T>(rect.bottom - rect.top));
			}
		}

		template <typename _T>
		Math::Rect<_T> GetFullRect(WindowHandle handle)
		{
			::RECT res{};
			::GetWindowRect((::HWND)handle, &res);

			return Cast<_T>(res);
		}

		template <typename _T>
		Math::Rect<_T> GetClientRect(WindowHandle handle)
		{
			::RECT res{};
			::GetClientRect((::HWND)handle, &res);

			return Cast<_T>(res);
		}

		bool IsVisible(WindowHandle handle)
		{
			return ::IsWindowVisible((::HWND)handle);
		}
	}
	
	namespace Console
	{
		template <EColourAttribute _A>
		void SetColourAttribute()
		{
			constexpr int value = static_cast<int>(_A);

			HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
			::SetConsoleTextAttribute(hConsole, value);
		}
	}
}

#endif // PLATFORM_WINDOWS
#endif