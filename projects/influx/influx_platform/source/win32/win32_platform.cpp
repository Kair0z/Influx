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
}