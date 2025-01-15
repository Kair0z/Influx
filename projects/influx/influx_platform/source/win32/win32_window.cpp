#include "win32_window.h"

// influx::core
#include "core/container/map.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()

namespace influx::platform
{
	static umap<::HWND, win32_window*> g_handle_to_window_map{};

	static window::rect translate(const ::RECT& rect)
	{
		return window::rect(
			static_cast<uint32>(rect.left),
			static_cast<uint32>(rect.bottom),
			static_cast<uint32>(rect.right - rect.left),
			static_cast<uint32>(rect.bottom - rect.top));
	}

	static window_event::type translate_umsg(uint32 uMsg)
	{
		switch (uMsg)
		{
		case WM_DESTROY: return window_event::type::quit;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: return window_event::type::keydown;
		case WM_SYSKEYUP:
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

	uint64 win32_window::window_proc(window_handle handle, uint32 message, uint64 wParam, uint64 lParam)
	{
		win32_window* target_window = nullptr;
		if (g_handle_to_window_map.contains((::HWND)handle))
		{
			target_window = g_handle_to_window_map[(::HWND)handle];
		}

		if (target_window)
		{
			window_event new_event{};
			new_event.m_mssg = message;
			new_event.m_wParam = wParam;
			new_event.m_lParam = lParam;
			new_event.m_type = translate_umsg(message);

			for (const event_callback& callback : target_window->m_event_callbacks)
			{
				if (callback)
				{
					callback(new_event);
				}
			}
		}

		switch (message)
		{
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			if (target_window)
			{
				target_window->request_quit();
			}
			return 0;
		}
		}

		return 0;
	}

	inline LRESULT _window_proc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		win32_window::window_proc(hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	window* window::create(const window_desc& desc)
	{
		return new win32_window(desc);
	}

	win32_window::win32_window(const window_desc& desc)
		: window(desc)
	{
		::HINSTANCE instance = (::HINSTANCE)platform::get_current_instance();

		const wstring wname = to_wstring(desc.m_name);
		const uint32 width = desc.m_dimensions.x;
		const uint32 height = desc.m_dimensions.y;

		// [ REGISTER WINDOW CLASS ]
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			::UINT class_style{};
			::HBRUSH classBackgroundBrush = ::CreateSolidBrush(0x00000000);

			::WNDCLASSEXW windowClassExtended;
			windowClassExtended.cbSize = sizeof(WNDCLASSEXW);
			windowClassExtended.style = class_style;
			windowClassExtended.lpfnWndProc = _window_proc;
			windowClassExtended.cbClsExtra = 0;
			windowClassExtended.cbWndExtra = 0;
			windowClassExtended.hInstance = instance;
			windowClassExtended.hIcon = NULL;
			windowClassExtended.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			windowClassExtended.hbrBackground = classBackgroundBrush;
			windowClassExtended.lpszMenuName = NULL;
			windowClassExtended.lpszClassName = wname.c_str();
			windowClassExtended.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

			influx_assert(::RegisterClassExW(&windowClassExtended));
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
			int xPos = (::GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2);
			int yPos = (::GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2);

			RECT clientArea;
			clientArea.left = xPos;
			clientArea.top = yPos;
			clientArea.right = xPos + width;
			clientArea.bottom = yPos + height;
			::AdjustWindowRect(&clientArea, windowStyle, FALSE);

			::HWND parentWindow = NULL;
			::HMENU parentMenu = NULL;

			newWindowHandle = ::CreateWindowExW(
				extendedWindowStyle,
				wname.c_str(),
				wname.c_str(),
				windowStyle,
				xPos, yPos, width, height,
				parentWindow, parentMenu, instance, NULL);

			influx_assert(newWindowHandle != NULL);

			m_handle = newWindowHandle;
		}

		// shouldnt been made yet...
		influx_assert(!g_handle_to_window_map.contains(newWindowHandle));
		g_handle_to_window_map[newWindowHandle] = this;

		// set default open
		set_visibility(e_visibility::showed);

		set_dimensions(desc.m_dimensions);
	}

	void win32_window::set_visibility(e_visibility vis)
	{
		::HWND native = (::HWND)m_handle;

		switch (vis)
		{
		case e_visibility::minimized:
			::CloseWindow(native);
			break;

		case e_visibility::showed:
			::ShowWindow(native, SW_SHOWNORMAL);
			break;

		case e_visibility::maximized:
			::ShowWindow(native, SW_SHOWMAXIMIZED);
			break;
		}
	}

	void win32_window::poll_events(bool& is_quit) const
	{
		vector<window_event::type> events{};

		// http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-4
		MSG msg;
		events.clear();
		events.reserve(16u); // how many could this really be xD?
		is_quit = false;

		// process ALL windows event message
		while (::PeekMessage(&msg, (::HWND)m_handle, 0u, 0u, PM_REMOVE))
		{
			events.push_back(translate_umsg(msg.message));

			if (events.back() == window_event::type::quit)
			{
				is_quit = true;
				break;
			}

			::TranslateMessage(&msg);

			// Dispatch to the WndProc
			::DispatchMessage(&msg);
		}
	}

	window_handle win32_window::get_platform_handle() const
	{
		return m_handle;
	}

	void win32_window::set_dimensions(const math::vectoru2& dimensions)
	{
		::HWND handle = (::HWND)m_handle;

		m_previous_dimensions_full = get_dimensions(e_space::full);
		m_previous_dimensions_client = get_dimensions(e_space::client);

		// Get the current window rectangle
		RECT rect;
		if (GetWindowRect(handle, &rect))
		{
			// Calculate current position
			int x = rect.left;
			int y = rect.top;

			bool res = ::SetWindowPos(
				handle,				// Handle to the window
				0,					// Z-order placement (NULL or special values)
				x,					// New X-coordinate of the top-left corner
				y,					// New Y-coordinate of the top-left corner
				dimensions.x,       // New width
				dimensions.y,       // New height
				0u					// Flags for window positioning
			);
		}

		m_current_dimensions_client = get_dimensions(e_space::client);
		m_current_dimensions_full = get_dimensions(e_space::full);
	}

	math::vectoru2 win32_window::get_dimensions(e_space space) const
	{
		return get_rect(space).get_dimensions();
	}

	math::vectoru2 win32_window::get_previous_dimensions(e_space space) const
	{
		switch (space)
		{
		case e_space::client:	return m_previous_dimensions_client;
		case e_space::full:		return m_previous_dimensions_full;
		}

		return {};
	}

	window::rect win32_window::get_rect(e_space space) const
	{
		::RECT res{};

		switch (space)
		{
		case e_space::client:
			::GetClientRect((::HWND)m_handle, &res);
			break;
		case e_space::full: 		
			::GetWindowRect((::HWND)m_handle, &res); 
			break;
		}

		return translate(res);
	}

	void win32_window::set_event_callback(const event_callback& callback)
	{
		m_event_callbacks.push_back(callback);
	}

	win32_window::~win32_window()
	{
		::DestroyWindow((::HWND)m_handle);
	}


	window_event::key_type window_event::parse_key_type() const
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
		case VK_MENU:
		case VK_LMENU:	return key_type::lalt;
		case VK_RMENU: return key_type::ralt;
		case VK_SPACE: return key_type::space;
		case VK_BACK: return key_type::backspace;
		case VK_RETURN: return key_type::enter;
		case VK_F2: return key_type::f2;
		default: return key_type::unknown;
		}
	}

	char window_event::parse_ascii() const
	{
		return (char)m_wParam;
	}

	float window_event::parse_wheel_delta() const
	{
		return (float)GET_WHEEL_DELTA_WPARAM(m_wParam) / WHEEL_DELTA;
	}

	math::vectorf2 window_event::parse_position_window() const
	{
		POINT mouse_pos = { (LONG)GET_X_LPARAM(m_lParam), (LONG)GET_Y_LPARAM(m_lParam) };

		// transform if necessary
		if (m_mssg == WM_NCMOUSEMOVE && m_window != nullptr)
		{
			::ScreenToClient((HWND)m_window->get_platform_handle(), &mouse_pos);
		}

		return { (float)mouse_pos.x, (float)mouse_pos.y };
	}

	math::vectorf2 window_event::parse_position_screen() const
	{
		POINT mouse_pos = { (LONG)GET_X_LPARAM(m_lParam), (LONG)GET_Y_LPARAM(m_lParam) };

		// transform if necessary
		if (m_mssg == WM_MOUSEMOVE && m_window != nullptr)
		{
			::ClientToScreen((HWND)m_window->get_platform_handle(), &mouse_pos);
		}

		return { (float)mouse_pos.x, (float)mouse_pos.y };
	}

	bool window_event::is_mouse_event() const
	{
		int type_value = (int)m_type;
		return
			type_value > (int)type::_mouse_begin &&
			type_value < (int)type::_mouse_end;
	}

	bool window_event::is_key_event() const
	{
		int type_value = (int)m_type;
		return
			type_value > (int)type::_key_begin &&
			type_value < (int)type::_key_end;
	}

	bool window_event::is_wheel_event() const
	{
		return
			m_type == type::wheel;
	}

	window_event::mouse_button window_event::parse_mouse_button() const
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
}