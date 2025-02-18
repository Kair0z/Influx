#include "win32_platform.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace influx::platform
{
	instance_handle platform::get_current_instance()
	{
		return ::GetModuleHandleW(NULL);
	}

	string platform::get_current_directory()
	{
		wchar_t buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);

		return to_string(buff);
	}

	void platform::set_current_directory(const string& path)
	{
		::SetCurrentDirectory(to_wstring(path).c_str());
	}

	math::vectoru2 platform::get_cursor_screenpos()
	{
		POINT mouse_screen_pos;
		bool has_mouse_screen_pos = ::GetCursorPos(&mouse_screen_pos) != 0;

		return math::vectoru2(mouse_screen_pos.x, mouse_screen_pos.y);
	}

	window_handle platform::find_window_from_cursor()
	{
		POINT mouse_screen_pos;
		bool has_mouse_screen_pos = ::GetCursorPos(&mouse_screen_pos) != 0;
		if (has_mouse_screen_pos)
		{
			return (window_handle)::WindowFromPoint(mouse_screen_pos);
		}
		
		return nullptr;
	}
}