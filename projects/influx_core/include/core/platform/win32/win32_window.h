#pragma once

#include "core/platform/window.h"
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

	inline window_handle create_window(const create_window_args& args, windows_procedure procedure_override)
	{
		// default open
		const bool make_open = true;

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
				messagebox(e_messagebox::error, "Fatal Error!", "Cannot Register Class", nullptr);
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
				messagebox(e_messagebox::error, "Fatal Error", "Failed Creating Window!", NULL);
				return nullptr;
			}
		}

		if (make_open)
		{
			set_window_visible(newWindowHandle, e_window_visibility::showed);
		}
		else
		{
			set_window_visible(newWindowHandle, e_window_visibility::minimized);
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
				messagebox(e_messagebox::warning, "Warning", "Failed RegisterRawInputDevices()!", nullptr);
			}
		}

		return newWindowHandle;
	}

	inline window_handle create_window(const create_window_args& args)
	{
		return create_window(args, detail::default_windows_procedure);
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
			
		case e_window_visibility::minimized:
			return ::CloseWindow(hwnd);
			break;

		case e_window_visibility::showed:
			return ::ShowWindow(hwnd, SW_SHOWNORMAL);
			break;

		case e_window_visibility::maximized:
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


	inline void messagebox(e_messagebox type, const string& caption, const string& message, const window_handle handle)
	{
		uint8 type_val = 0u;
		switch (type)
		{
		default:
		case e_messagebox::info:
			type_val = MB_ICONINFORMATION | MB_OK;
			break;

		case e_messagebox::warning:
			type_val = MB_ICONWARNING | MB_OK;
			break;

		case e_messagebox::error:
			type_val = MB_ICONEXCLAMATION | MB_OK;
			break;
		}

		::MessageBoxA((::HWND)handle, message.c_str(), caption.c_str(), type_val);
	}
}