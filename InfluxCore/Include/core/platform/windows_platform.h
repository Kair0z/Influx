#pragma once

#ifndef __CORE_WINDOWSPLATFORM_H_
#define __CORE_WINDOWSPLATFORM_H_

#include "core/platform/platform.h"
#include "core/basetypes.h"
#include "core/cast.h"
#include "core/container/containers.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// We don't like UNICODE steered macros! 
// We will always use the UNICODE A versions of these functions!
#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef messagebox
#undef message_box
#endif

namespace influx::platform
{
	using windows_procedure = ::WNDPROC;

	namespace detail
	{
		static list<window_handle>		gWindowHandles{};
		static list<windows_procedure>	gWindowsProcedureCallbacklist{};

		constexpr e_windowevent translate_event(const uint32 value)
		{
			switch (value)
			{
			default:
			case WM_NULL:		return e_windowevent::unknown;
			case WM_QUIT:		return e_windowevent::quit;
			case WM_ACTIVATE:	return e_windowevent::activate;
			}
		}

		inline LRESULT default_windows_procedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
		{
			for (const windows_procedure& proc : gWindowsProcedureCallbacklist)
			{
				proc(hWnd, uMsg, wParam, lParam);
			}

			switch (uMsg)
			{
			case WM_DESTROY:
			{
				::PostQuitMessage(0);
				return 0;
			}

			default:
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		template <typename _t>
		inline math::rect<_t> cast(const ::RECT& rect)
		{
			return math::rect<_t>(
				stat_cast<_t>(rect.left),
				stat_cast<_t>(rect.bottom),
				stat_cast<_t>(rect.right - rect.left),
				stat_cast<_t>(rect.bottom - rect.top));
		}
	}

	// [ MEMORY ]
	inline void* allocate(const uint64 size)
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

	template <typename _t>
	inline _t* allocate()
	{
		return static_cast<_t*>(allocate(sizeof(_t)));
	}

	template <typename _t, typename ..._args>
	inline _t* anew(_args&&... args)
	{
		return new _t(args...);
	}

	template <typename _t>
	inline void free(_t* address)
	{
		std::free(address);
	}

	// [ APPLICATION ]
#pragma region application
	inline process_handle get_current_process()
	{
		return ::GetCurrentProcess();
	}

	inline instance_handle get_current_instance()
	{
		return ::GetModuleHandleW(NULL);
	}

	inline window_handle get_current_window()
	{
		return ::GetActiveWindow();
	}

	inline bool is_window_valid(window_handle handle)
	{
		return ::IsWindow((::HWND)handle);
	}

	inline void quit()
	{
		::PostQuitMessage(0);
	}
#pragma endregion

	// [ WINDOW ]
#pragma region window

	/* Returns false if a quit-event was polled! */
	inline bool poll_window_events(vector<e_windowevent>& out_events, window_handle handle = get_current_window())
	{
		// http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-4
		MSG msg;
		if (!is_window_valid(handle))
		{
			return false;
		}
		
		bool found_quit_event = false;
		out_events.clear();
		out_events.reserve(16u); // how many could this really be xD?

		// process ALL windows event message
		while (::PeekMessage(&msg, (::HWND)handle, 0u, 0u, PM_REMOVE))
		{
			e_windowevent translatedEvent = detail::translate_event(msg.message);
			out_events.push_back(translatedEvent);

			if (translatedEvent == e_windowevent::quit)
			{
				found_quit_event = true;
				break;
			}

			::TranslateMessage(&msg);

			// Dispatch to the WndProc
			::DispatchMessage(&msg);
		}

		return !found_quit_event;
	}

	/* Returns false if a quit-event was polled! */
	inline bool poll_window_events(window_handle handle = get_current_window())
	{
		vector<e_windowevent> out_events{};
		return poll_window_events(out_events, handle);
	}

	inline window_handle create_window(const create_window_args& args, bool make_open, windows_procedure procedure_override = detail::default_windows_procedure)
	{
		::HINSTANCE instance = (::HINSTANCE)get_current_instance();
		const wstring nameWstring = to_wstring(args.m_name);

		// [ REGISTER WINDOW CLASS ]
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			::UINT windowClassStyle{};
			::HBRUSH classBackgroundBrush = ::CreateSolidBrush(0x00000000);

			::WNDCLASSEXW windowClassExtended;
			windowClassExtended.cbSize			= sizeof(WNDCLASSEXW);
			windowClassExtended.style			= windowClassStyle;
			windowClassExtended.lpfnWndProc		= procedure_override;
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
				messagebox_error("Fatal Error!", "Cannot Register Class", nullptr);
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
			int xPos = (::GetSystemMetrics(SM_CXSCREEN) / 2) - (args.m_width / 2);
			int yPos = (::GetSystemMetrics(SM_CYSCREEN) / 2) - (args.m_height / 2);

			RECT clientArea;
			clientArea.left		= xPos;
			clientArea.top		= yPos;
			clientArea.right	= xPos + args.m_width;
			clientArea.bottom	= yPos + args.m_height;
			::AdjustWindowRect(&clientArea, windowStyle, FALSE);

			::HWND parentWindow = NULL;
			::HMENU parentMenu = NULL;

			newWindowHandle = ::CreateWindowExW(
				extendedWindowStyle,
				nameWstring.c_str(),
				nameWstring.c_str(),
				windowStyle,
				xPos, yPos, args.m_width, args.m_height,
				parentWindow, parentMenu, instance, NULL);

			if (newWindowHandle == NULL)
			{
				messagebox_error("Fatal Error", "Failed Creating Window!", NULL);
				return nullptr;
			}
		}

		if (make_open)
		{
			set_window_visible(newWindowHandle, e_window_visibility::ShowDefault);
		}
		else
		{
			set_window_visible(newWindowHandle, e_window_visibility::Minimize);
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
				messagebox_warning("Warning", "Failed RegisterRawInputDevices()!", nullptr);
			}
		}

		return newWindowHandle;
	}

	inline window_handle create_window(const create_window_args& args, bool make_open)
	{
		return create_window(args, make_open, detail::default_windows_procedure);
	}

	inline void destroy_window(const window_handle handle)
	{
		::DestroyWindow((::HWND)handle);
	}

	inline bool set_window_visible(const window_handle windowHandle, const e_window_visibility command)
	{
		::HWND hwnd = (::HWND)windowHandle;

		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
		switch (command)
		{
		default:
			return false;
			
		case e_window_visibility::Minimize:
			return ::CloseWindow(hwnd);
			break;

		case e_window_visibility::ShowDefault:
			return ::ShowWindow(hwnd, SW_SHOWNORMAL);
			break;

		case e_window_visibility::Maximize:
			return ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
			break;
		}
	}

	template <typename _t>
	inline math::rect<_t> get_windowrect_full(const window_handle windowHandle)
	{
		::RECT res{};
		::GetWindowRect((::HWND)windowHandle, &res);

		return detail::cast<_t>(res);
	}

	template <typename _t>
	inline math::rect<_t> get_windowrect_client(const window_handle windowHandle)
	{
		::RECT res{};
		::GetClientRect((::HWND)windowHandle, &res);

		return detail::cast<_t>(res);
	}

	inline bool is_window_visible(const window_handle windowHandle)
	{
		return ::IsWindowVisible((::HWND)windowHandle);
	}
#pragma endregion

	// [ MISC ]
	template <e_messagebox _t>
	inline void messagebox(const string& caption, const string& message, const window_handle windowHandle)
	{
		uint8 type = 0u;
		switch (_t)
		{
		default:
		case e_messagebox::info:
			type = MB_ICONINFORMATION | MB_OK;
			break;

		case e_messagebox::warning:
			type = MB_ICONWARNING | MB_OK;
			break;

		case e_messagebox::error:
			type = MB_ICONEXCLAMATION | MB_OK;
			break;
		}
		
		::MessageBoxA((::HWND)windowHandle, message.c_str(), caption.c_str(), type);
	}

	template <e_console_colour _C>
	inline void set_console_colour_attribute()
	{
		constexpr int value = static_cast<int>(_C);

		HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::SetConsoleTextAttribute(hConsole, value);
	}

#pragma region files
	inline string get_current_directory()
	{
		wchar_t buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);

		return to_string(buff);
	}

	inline void set_current_directory(const string& path)
	{
		::SetCurrentDirectory(to_wstring(path).c_str());
	}
#pragma endregion
}

#endif // PLATFORM_WINDOWS
