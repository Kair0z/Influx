#include "win32_window.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()

#include "core/container/map.h"

namespace influx::platform
{
	static umap<::HWND, win32_window*> g_handle_to_window_map{};

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

	inline LRESULT window_proc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		// default behaviour
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			if (g_handle_to_window_map.contains(hWnd))
				g_handle_to_window_map[hWnd]->request_quit();
			return 0;
		}
		}

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
			windowClassExtended.lpfnWndProc = window_proc;
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

	win32_window::~win32_window()
	{
		::DestroyWindow((::HWND)m_handle);
	}
}