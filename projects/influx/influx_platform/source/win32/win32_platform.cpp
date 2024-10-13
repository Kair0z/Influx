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

	const string& platform::get_current_directory()
	{
		wchar_t buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);

		return to_string(buff);
	}

	void platform::set_current_directory(const string& path)
	{
		::SetCurrentDirectory(to_wstring(path).c_str());
	}
}