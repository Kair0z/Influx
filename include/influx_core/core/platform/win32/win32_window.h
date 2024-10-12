#pragma once

#include "core/platform/window.h"
#include "core/basetypes.h"
#include "core/cast.h"
#include "core/container/containers.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include "core/platform/window.h"

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
	inline window_event::key_type window_event::parse_key_type() const
	{
		/*
		* VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
		* 0x3A - 0x40 : unassigned
		* VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A) */
		const bool is_ascii_number = m_wParam >= 0x30 && m_wParam <= 0x39;
		if (is_ascii_number)
		{
			return key_type::ascii_num;
		}
		const bool is_ascii_character = m_wParam >= 0x41 && m_wParam <= 0x5A;
		if (is_ascii_character)
		{
			return key_type::ascii_ch;
		}

		switch (m_wParam)
		{
		case VK_LEFT: return key_type::left;
		case VK_RIGHT: return key_type::right;
		case VK_UP: return key_type::up;
		case VK_DOWN: return key_type::down;
		case VK_HOME: return key_type::home;
		case VK_END: return key_type::end;
		case VK_INSERT: return key_type::insert;
		case VK_DELETE: return key_type::deleet;
		case VK_SHIFT: return key_type::lshift;
		case VK_RSHIFT: return key_type::rshift;
		case VK_CONTROL: return key_type::lctrl;
		case VK_RCONTROL: return key_type::rctrl;
		case VK_SPACE: return key_type::space;
		case VK_F2: return key_type::f2;
		default: return key_type::unknown;
		}
	}

	inline char window_event::parse_ascii() const
	{
		return (char)m_wParam;
	}

	inline float window_event::parse_wheel_delta() const
	{
		return (float)GET_WHEEL_DELTA_WPARAM(m_wParam) / WHEEL_DELTA;
	}

	inline math::vectorf2 window_event::parse_position_window() const
	{
		POINT mouse_pos = { (LONG)GET_X_LPARAM(m_lParam), (LONG)GET_Y_LPARAM(m_lParam) };

		// transform if necessary
		if (m_mssg == WM_NCMOUSEMOVE)
		{
			::ScreenToClient((HWND)get_current_window(), &mouse_pos);
		}

		return { (float)mouse_pos.x, (float)mouse_pos.y };
	}

	inline math::vectorf2 window_event::parse_position_screen() const
	{
		POINT mouse_pos = { (LONG)GET_X_LPARAM(m_lParam), (LONG)GET_Y_LPARAM(m_lParam) };

		// transform if necessary
		if (m_mssg == WM_MOUSEMOVE)
		{
			::ClientToScreen((HWND)get_current_window(), &mouse_pos);
		}

		return { (float)mouse_pos.x, (float)mouse_pos.y };
	}

	inline window_event::mouse_button window_event::parse_mouse_button() const
	{
		switch (m_mssg)
		{
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: case WM_LBUTTONUP:
			return window_event::mouse_button::left;

		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: case WM_RBUTTONUP:
			return window_event::mouse_button::right;

		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: case  WM_MBUTTONUP:
			return window_event::mouse_button::middle;

		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: case WM_XBUTTONUP:
			return window_event::mouse_button::x;
		}

		return window_event::mouse_button::count;
	}
	namespace detail
	{
		static list<window_handle>		gWindowHandles{};
		static list<window_proc_callback>	g_windows_procs{};

		static window_event::type translate_umsg(uint32 uMsg)
		{
			switch (uMsg)
			{
			case WM_DESTROY: return window_event::type::quit;
			case WM_KEYDOWN: return window_event::type::keydown;
			case WM_KEYUP: return window_event::type::keyup;
			case WM_MOUSEWHEEL: return window_event::type::wheel;

			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE: 
				return window_event::type::mouse_move;

			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE: 
				return window_event::type::mouse_leave;

			case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
				return window_event::type::mouse_down;

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
				return window_event::type::mouse_up;
			}

			return window_event::type::count;
		}

		static window_event make_window_event(uint32 uMsg, uint64 wParam, uint64 lParam)
		{
			window_event ev{};

			ev.m_wParam = wParam;
			ev.m_lParam = lParam;
			ev.m_mssg = uMsg;

			// determine type
			ev.m_type = translate_umsg(uMsg);
			return ev;
		}

		inline LRESULT influx_window_proc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
		{
			// call user procedures
			for (const window_proc_callback& callback : g_windows_procs)
			{
				if (callback)
				{
					window_event ev = make_window_event(uMsg, wParam, lParam);
					callback(ev);
				}
			}

			// default behaviour
			switch (uMsg)
			{
				case WM_DESTROY:
				{
					::PostQuitMessage(0);
					return 0;
				}
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
	inline bool poll_window_events(vector<window_event::type>& out_events, window_handle handle = get_current_window())
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
			window_event::type translated_type = detail::translate_umsg(msg.message);
			out_events.push_back(translated_type);

			if (translated_type == window_event::type::quit)
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
		vector<window_event::type> out_events{};
		return poll_window_events(out_events, handle);
	}

	inline window_handle create_window(const create_window_args& args)
	{
		// default open
		const bool make_open = true;

		::HINSTANCE instance = (::HINSTANCE)get_current_instance();
		const wstring nameWstring = to_wstring(args.m_name);

		detail::g_windows_procs.push_back(args.m_proc_callback);

		// [ REGISTER WINDOW CLASS ]
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			::UINT windowClassStyle{};
			::HBRUSH classBackgroundBrush = ::CreateSolidBrush(0x00000000);

			::WNDCLASSEXW windowClassExtended;
			windowClassExtended.cbSize			= sizeof(WNDCLASSEXW);
			windowClassExtended.style			= windowClassStyle;
			windowClassExtended.lpfnWndProc		= detail::influx_window_proc;
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

	inline void add_window_proc(const window_handle handle, const window_proc_callback& callback)
	{
		detail::g_windows_procs.push_back(callback);
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