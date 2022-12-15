#pragma once

#ifndef _CORE_PLATFORM_WINDOWS_H_
#define _CORE_PLATFORM_WINDOWS_H_

#include "Core/Geometry/Rect.h"
#include "Core/String.h"
#include "Core/Cast.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace Influx::Platform
{
	namespace Internal
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

	inline ::HANDLE GetCurrentProcess()
	{
		return ::GetCurrentProcess();
	}

	inline ::HINSTANCE GetCurrentInstance()
	{
		return ::GetModuleHandleW(NULL);
	}

	inline void Quit(int quitMessageId)
	{
		::PostQuitMessage(quitMessageId);
	}

	struct DefaultWindowsProcedure final
	{
		using OnDestroyWindowClb = void(*)();
		using OnCloseWindowClb = void(*)(::HWND);

		static OnDestroyWindowClb s_onDestroyWindow;
		static OnCloseWindowClb s_onCloseWindow;
		static inline LRESULT CALLBACK Procedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_CLOSE:
				if (s_onCloseWindow != nullptr) s_onDestroyWindow();
				break;
			case WM_DESTROY:
				if (s_onDestroyWindow != nullptr) s_onCloseWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
			return 0;
		}
	};
	
	inline DefaultWindowsProcedure::OnCloseWindowClb DefaultWindowsProcedure::s_onCloseWindow = nullptr;
	inline DefaultWindowsProcedure::OnDestroyWindowClb DefaultWindowsProcedure::s_onDestroyWindow = nullptr;

	inline ::HWND CreateWindow(int w, int h, const WString& name, ::WNDPROC winProc = DefaultWindowsProcedure::Procedure, ::HINSTANCE i = GetCurrentInstance())
	{
		// prepare window class
		WNDCLASSEXW wc;

		if (winProc == NULL) winProc = DefWindowProcA;
		if (i == NULL) i = GetModuleHandle(NULL);

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = winProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = i;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)CreateSolidBrush(0x00000000);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = name.c_str();
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		// register window class
		if (!RegisterClassExW(&wc))
		{
			MessageBox(NULL, L"Couldn't RegisterClassEx()!", L"Fatal Error", MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}

		/* Window Handle */
		HWND hwnd;

		LONG_PTR style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		//style = style & (~WS_SIZEBOX);

#ifdef WINDOW_FRAMELESS
		style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION;
#endif
		LONG_PTR exStyle = WS_EX_WINDOWEDGE;

		style &= ~WS_VISIBLE;

		int xPos = (GetSystemMetrics(SM_CXSCREEN) / 2) - (w / 2);
		int yPos = (GetSystemMetrics(SM_CYSCREEN) / 2) - (h / 2);
		int width = w;
		int height = h;

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

		hwnd = CreateWindowExW((DWORD)exStyle,
			name.c_str(),
			name.c_str(),
			(DWORD)style,
			xPos, yPos, width, height,
			NULL, NULL, i, NULL);

		if (hwnd == NULL)
		{
			MessageBox(NULL, L"Error: Failed Creating Window!", L"Fatal Error", MB_ICONEXCLAMATION | MB_OK);
			return NULL;
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

	inline ::HANDLE LaunchWindowsThread(::LPTHREAD_START_ROUTINE func)
	{
		return ::CreateThread(NULL, 0, func, NULL, 0, NULL);
	}

	inline ::HMODULE LoadModule(const WString& moduleName)
	{
		return LoadLibrary(moduleName.c_str());
	}

	template <typename _T>
	inline Math::Rect<_T> GetWindowFullRect(HWND handle = GetCurrentWindowHandle())
	{
		::RECT res{};
		::GetWindowRect(handle, &res);

		return Internal::Cast(res);
	}

	template <typename _T>
	inline  Math::Rect<_T> GetWindowClientRect(HWND handle = GetCurrentWindowHandle())
	{
		::RECT res{};
		::GetClientRect(handle, &res);

		return Internal::Cast<_T>(res);
	}

	inline ::HWND GetCurrentWindowHandle()
	{
		return ::GetActiveWindow();
	}

	inline bool IsWindowVisible(::HWND handle = GetCurrentWindowHandle())
	{
		return ::IsWindowVisible(handle);
	}

	inline void DestroyWindow(::HWND handle)
	{
		::DestroyWindow(handle);
	}

	enum class EConsoleColour
	{
		Green	= 2,
		Red		= 4,
		Purple	= 5
	};

	template <EConsoleColour _C>
	void SetConsoleColourTextAttribute(bool backgroundVersion = false)
	{
		constexpr size_t value = static_cast<size_t>(_C);

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, backgroundVersion ? value + 10u : value);
	}
}

#endif