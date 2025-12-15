#include "win32_window.h"

// influx::core
#include "core/container/map.h"
#include "core/log.h"

// influx::platform
#include "influx_platform/monitor.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()

namespace influx::platform
{
	static umap<::HWND, win32_window*> g_handle_to_window_map{};

	static ::RECT translate(const window::rect& rect)
	{
		return
		{
			(LONG)rect.get_left(),
			(LONG)rect.get_bottom(), 
			(LONG)rect.get_width(), 
			(LONG)rect.get_height()
		};
	}

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
		
		case WM_CLOSE: return window_event::type::close;
		case WM_MOVE: return window_event::type::move;
		case WM_SIZE: return window_event::type::size;
		case WM_MOUSEACTIVATE: return window_event::type::mouse_activate;

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

	inline static string parse_error(DWORD errorcode)
	{
		WCHAR* messageBuffer = nullptr;

		// Format the error message from the system
		size_t size = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&messageBuffer, 0, NULL);

		std::wstring message(messageBuffer, size);
		LocalFree(messageBuffer); // Free the buffer allocated by FormatMessage
		
		return to_string(message);
	}

	inline static math::vectoru2 get_dimensions(::HWND handle)
	{
		::RECT res{};
		::GetWindowRect(handle, &res);
		return translate(res).get_dimensions();
	}

	inline static string get_title(::HWND handle)
	{
		LPWSTR result{};
		::GetWindowTextW(handle, result, 500000u);
		return to_string(result);
	}

	inline static math::vectoru2 get_position(::HWND handle)
	{
		::RECT res{};
		::GetWindowRect(handle, &res);
		return translate(res).get_leftbottom();
	}

	inline static window_style get_style(::HWND handle)
	{
		window_style new_style{};
		new_style.m_style = ::GetWindowLong(handle, GWL_STYLE);
		new_style.m_style_ext = ::GetWindowLong(handle, GWL_EXSTYLE);
		return new_style;
	}

	inline static window_desc parse_desc(::HWND handle)
	{
		window_desc desc{};
		desc.m_dimensions = get_dimensions(handle);
		desc.m_name = get_title(handle);
		desc.m_position = get_position(handle);
		desc.m_style = get_style(handle);
		return desc;
	}

	uint64 win32_window::window_proc(window_handle handle, uint32 message, uint64 wParam, uint64 lParam)
	{
		DWORD error = ::GetLastError();
		if (error)
		{
			//const string error_str = parse_error(error);
			//OutputDebugString(to_wstring(error_str).c_str());
			//logwar(error_str);
		}

		//const string message_str = "mssg:" + to_string(message) + "\n";
		//OutputDebugString(to_wstring(message_str).c_str());

		::HWND native_handle = (::HWND)handle;
		win32_window* target_window = nullptr;
		if (g_handle_to_window_map.contains(native_handle))
		{
			target_window = g_handle_to_window_map[native_handle];
		}

		// target window callbacks
		if (target_window)
		{
			window_event new_event{};
			new_event.m_mssg = message;
			new_event.m_wParam = wParam;
			new_event.m_lParam = lParam;
			new_event.m_type = translate_umsg(message);
			new_event.m_window = target_window;

			for (const event_callback& callback : target_window->m_event_callbacks)
			{
				if (callback) callback(new_event);
			}
		}

		// default callbacks
		switch (message)
		{
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			if (target_window)
			{
				target_window->request_quit();
			}
		}
		default: { return ::DefWindowProc(native_handle, message, wParam, lParam); };
		}
	}

	inline LRESULT _window_proc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		return win32_window::window_proc(hWnd, uMsg, wParam, lParam);
	}

	window* window::create(const window_desc& desc)
	{
		return new win32_window(desc);
	}

	window* window::import_window(const window_handle& handle)
	{
		return new win32_window(handle);
	}

	win32_window::win32_window(const window_desc& desc)
		: window(desc)
	{
		influx_assert(desc.m_name.empty() == false);
		::HINSTANCE instance = (::HINSTANCE)platform::get_current_instance();

		const string& name = desc.m_name;
		const uint32 width = desc.m_dimensions.x;
		const uint32 height = desc.m_dimensions.y;

		// [ REGISTER WINDOW CLASS ]
		static bool once = true;
		static umap<string, bool> onces{};
		if (onces[desc.m_name] == false)
		{
			onces[desc.m_name] = true;

			// https://learn.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			::HBRUSH classBackgroundBrush = (HBRUSH)(COLOR_BACKGROUND + 1);

			::WNDCLASSEX windowClassExtended;
			windowClassExtended.cbSize = sizeof(WNDCLASSEX);
			windowClassExtended.style = CS_HREDRAW | CS_VREDRAW;
			windowClassExtended.lpfnWndProc = _window_proc;
			windowClassExtended.cbClsExtra = 0;
			windowClassExtended.cbWndExtra = 0;
			windowClassExtended.hInstance = instance;
			windowClassExtended.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
			windowClassExtended.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			windowClassExtended.hbrBackground = classBackgroundBrush;
			windowClassExtended.lpszMenuName = NULL;
			windowClassExtended.lpszClassName = name.c_wstr();
			windowClassExtended.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

			auto res = ::RegisterClassEx(&windowClassExtended);
			if (!res)
			{
				logerr(parse_error(::GetLastError()));
				influx_assert(false);
			}
		}

		// [ CREATE WINDOW ]
		::HWND newWindowHandle = NULL;
		{
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
			::DWORD extendedWindowStyle = desc.m_style.m_style_ext;
			::DWORD windowStyle = desc.m_style.m_style;
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

			newWindowHandle = ::CreateWindowEx(
				extendedWindowStyle,
				name.c_wstr(),
				name.c_wstr(),
				windowStyle,
				xPos, yPos, width, height,
				parentWindow, parentMenu, instance, NULL);

			if (newWindowHandle == NULL)
			{
				logerr(parse_error(GetLastError()));
			}
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

	win32_window::win32_window(const window_handle& handle)
		: window( parse_desc( (::HWND)handle ))
	{
		HWND windowhandle = (HWND)handle;

		// shouldnt been made yet...
		influx_assert(!g_handle_to_window_map.contains(windowhandle));
		g_handle_to_window_map[windowhandle] = this;
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

		if (is_valid() == false)
		{
			is_quit = true;
			return;
		}

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

	result<e_messagebox_result> win32_window::messagebox(
		const string& caption,
		const string& message,
		const e_messagebox_flags flags,
		const e_messagebox_icon icon) const
	{
		using result_type = result<e_messagebox_result>;

		// figure out the buttons
		long winflags = MB_RETRYCANCEL;
		if (has_flag(flags, e_messagebox_flags::button_ok))
		{
			
		}
		if (has_flag(flags, e_messagebox_flags::button_cancel))
		{
			
		}

		// figure out the icon
		switch (icon)
		{
		case e_messagebox_icon::exclamation: winflags |= MB_ICONEXCLAMATION; break;
		case e_messagebox_icon::warning:	 winflags |= MB_ICONWARNING; break;
		case e_messagebox_icon::info:		 winflags |= MB_ICONINFORMATION; break;
		case e_messagebox_icon::asterisk:	 winflags |= MB_ICONASTERISK; break;
		case e_messagebox_icon::question:	 winflags |= MB_ICONQUESTION; break;
		case e_messagebox_icon::stop:		 winflags |= MB_ICONSTOP; break;
		case e_messagebox_icon::error:		 winflags |= MB_ICONERROR; break;
		case e_messagebox_icon::hand:		 winflags |= MB_ICONHAND; break;
		}

		int winresult = MessageBox(
			NULL,
			message.c_wstr(),
			caption.c_wstr(),
			winflags
		);

		// If the function fails, the return value is zero. 
		// To get extended error information, call GetLastError.
		if (winresult == 0)
			return result_type::make_error("windows messagebox failed!");
		else
		{
			switch (winresult)
			{
			default:
			case IDOK: return e_messagebox_result::ok;
			case IDABORT: return e_messagebox_result::abort;
			case IDCANCEL: return e_messagebox_result::cancel;
			case IDCONTINUE: return e_messagebox_result::continu;
			case IDIGNORE: return e_messagebox_result::ignore;
			case IDNO: return e_messagebox_result::no;
			case IDTRYAGAIN:
			case IDRETRY: return e_messagebox_result::retry; // 'try again'
			case IDYES: return e_messagebox_result::yes;
			}
		}
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
			bool res = ::SetWindowPos(
				handle,				// Handle to the window
				NULL,					// Z-order placement (NULL or special values)
				0,					// New X-coordinate of the top-left corner
				0,					// New Y-coordinate of the top-left corner
				dimensions.x,       // New width
				dimensions.y,       // New height
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE // Flags for window positioning
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

	void win32_window::set_position(const math::vectoru2& pos)
	{
		const math::vectoru2 full_dimensions = get_dimensions(e_space::full);
		RECT rect = { (LONG)pos.x, (LONG)pos.y, (LONG)full_dimensions.x, (LONG)full_dimensions.y };
		::AdjustWindowRectEx(&rect, m_desc.m_style.m_style, FALSE, m_desc.m_style.m_style_ext);
		::SetWindowPos((HWND)m_handle, nullptr, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	math::vectoru2 win32_window::get_position() const
	{
		POINT pos = { 0, 0 };
		::ClientToScreen((HWND)m_handle, &pos);
		return math::vectoru2((uint32)pos.x, (uint32)pos.y);
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

	void win32_window::set_title(const string& new_title)
	{
		::SetWindowTextW((HWND)m_handle, new_title.c_wstr());
	}
	bool win32_window::is_foreground() const
	{
		return ::GetForegroundWindow() == (HWND)m_handle;
	}
	void win32_window::set_foreground()
	{
		::BringWindowToTop((HWND)m_handle);
		::SetForegroundWindow((HWND)m_handle);
	}
	bool win32_window::is_focus() const
	{
		return ::GetFocus() == (HWND)m_handle;
	}
	void win32_window::set_focus()
	{
		::SetFocus((HWND)m_handle);
	}
	bool win32_window::is_minimized() const
	{
		return ::IsIconic((HWND)m_handle) != 0;
	}
	void win32_window::set_alpha(float alpha)
	{
		HWND handle = (HWND)m_handle;
		alpha = math::clamp(alpha, 0.0f, 1.0f);

		if (alpha < 1.0f)
		{
			DWORD ex_style = ::GetWindowLongW(handle, GWL_EXSTYLE) | WS_EX_LAYERED;
			::SetWindowLongW(handle, GWL_EXSTYLE, ex_style);
			::SetLayeredWindowAttributes(handle, 0, (BYTE)(255 * alpha), LWA_ALPHA);
		}
		else
		{
			DWORD ex_style = ::GetWindowLongW(handle, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			::SetWindowLongW(handle, GWL_EXSTYLE, ex_style);
		}
	}
	float win32_window::get_alpha() const
	{
		return 1.0f;
	}

	float win32_window::get_dpi() const
	{
		HMONITOR monitor = ::MonitorFromWindow((HWND)m_handle, MONITOR_DEFAULTTONEAREST);
		return 1.0f;
	}

	string win32_window::get_title() const
	{
		return m_desc.m_name;
	}

	window_style win32_window::get_style() const
	{
		return m_desc.m_style;
	}

	void win32_window::set_style(const window_style& new_style)
	{
		if (m_desc.m_style.m_style != new_style.m_style || 
			m_desc.m_style.m_style_ext != new_style.m_style_ext)
		{
			m_desc.m_style = new_style;

			::SetWindowLong((HWND)m_handle, GWL_STYLE, new_style.m_style);
			::SetWindowLong((HWND)m_handle, GWL_EXSTYLE, new_style.m_style_ext);

			::ShowWindow((HWND)m_handle, SW_SHOWNA); // This is necessary when we alter the style
		}
	}

	void win32_window::set_parent(window& parent)
	{
		if (m_parent != parent.get_platform_handle())
		{
			m_parent = parent.get_platform_handle();

			// Win32 windows can either have a "Parent" (for WS_CHILD window) or an "Owner" (which among other thing keeps window above its owner).
			// Our Dear Imgui-side concept of parenting only mostly care about what Win32 call "Owner".
			// The parent parameter of CreateWindowEx() sets up Parent OR Owner depending on WS_CHILD flag. In our case an Owner as we never use WS_CHILD.
			// Calling ::SetParent() here would be incorrect: it will create a full child relation, alter coordinate system and clipping.
			// Calling ::SetWindowLongPtr() with GWLP_HWNDPARENT seems correct although poorly documented.
			// https://devblogs.microsoft.com/oldnewthing/20100315-00/?p=14613
			::SetWindowLongPtr((HWND)m_handle, GWLP_HWNDPARENT, (LONG_PTR)parent.get_platform_handle());
		}
	}

	void win32_window::set_owner(window& owner)
	{
		set_parent(owner);
	}

	window::rect win32_window::adjust_rect(const rect& rect)
	{
		RECT Rect = translate(rect);
		::AdjustWindowRectEx(&Rect, m_desc.m_style.m_style, FALSE, m_desc.m_style.m_style_ext);
		return translate(Rect);
	}

	bool win32_window::is_valid() const
	{
		return ::IsWindow((HWND)m_handle);
	}

	void win32_window::set_event_callback(const event_callback& callback)
	{
		m_event_callbacks.push_back(callback);
	}

	win32_window::~win32_window()
	{
		if (g_handle_to_window_map.contains((::HWND)m_handle))
			g_handle_to_window_map.erase((::HWND)m_handle);

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
		case VK_OEM_1: return key_type::semicolon;
		case VK_OEM_PLUS: return key_type::plus;
		case VK_OEM_COMMA: return key_type::comma;
		case VK_OEM_MINUS: return key_type::minus;
		case VK_OEM_PERIOD: return key_type::period;
		case VK_OEM_4: return key_type::lbracket;
		case VK_OEM_6: return key_type::rbracket;
		case VK_OEM_5: return key_type::backslash;
		case VK_OEM_7: return key_type::apostrophe;
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

	math::vectorf2 window_event::parse_position_window_normalized() const
	{
		const auto rect = m_window->get_rect(window::e_space::client);
		math::vectorf2 position = parse_position_window();
		position.x /= rect.get_width();
		position.y /= rect.get_height();
		return position;
	}
	math::vectorf2 window_event::parse_position_screen_normalized() const
	{
		monitor current_mon = monitor::from_window(*m_window);
		const auto screen_rect = current_mon.get_rect();

		math::vectorf2 position = parse_position_screen();
		position.x /= screen_rect.get_width();
		position.y /= screen_rect.get_height();
		return position;
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

	void window_style::set_decoration(bool enabled)
	{
		if (enabled)
		{
			m_style = WS_OVERLAPPEDWINDOW;
		}
		else
		{
			m_style = WS_POPUP;
		}
	}

	bool window_style::get_decoration() const
	{
		return m_style & WS_OVERLAPPEDWINDOW;
	}

	void window_style::set_taskicon_enabled(bool enabled)
	{
		if (enabled)
		{
			m_style_ext |= WS_EX_TOOLWINDOW;
		}
		else
		{
			m_style_ext |= WS_EX_APPWINDOW;
		}
	}

	bool window_style::get_taskicon_enabled() const
	{
		return m_style_ext & WS_EX_APPWINDOW;
	}

	void window_style::set_topmost(bool enabled)
	{
		if (enabled)
		{
			m_style_ext |= WS_EX_TOPMOST;
		}
	}

	void window_style::set_generic_window(bool enabled)
	{
		if (enabled)
		{
			m_style = WS_OVERLAPPEDWINDOW;
		}
	}

	void window_style::set_exit_button(bool enabled)
	{
		if (enabled)
		{
			m_style |= WS_SYSMENU;
		}
		else
		{
			m_style &= ~WS_SYSMENU;
		}
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