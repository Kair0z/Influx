#pragma once

#if 0
// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/string.h"
#include "core/function.h"
#include "core/singleton.h"

// influx::platform
#include "core/platform/platform_common.h"

namespace influx::platform
{
	// [instance]
	process_handle get_current_process();

	instance_handle get_current_instance();

	void quit();

	void set_console_colour_attribute(e_console_colour colour);

	string get_current_directory();

	void set_current_directory(const string& path);
}
#endif