#pragma once
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/string.h"
#include "core/function.h"
#include "core/singleton.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace influx::platform
{
#pragma region types
	using process_handle = void*;
	using instance_handle = void*;
	using event_handle = void*;

	enum class e_console_colour : uint16
	{
		blue		= 1,
		green		= 2,
		red			= 4,
		yellow		= green | red,
		purple		= red | blue,
		white		= green | red | blue,
		bg_blue		= 11,
		bg_green	= 12,
		bg_red		= 14,
		bg_purple	= bg_red | bg_blue,
		count
	};
#pragma endregion

	// [instance]
	process_handle get_current_process();

	instance_handle get_current_instance();

	void quit();

	// [miscelaneous]
	void set_console_colour_attribute(e_console_colour colour);

	string get_current_directory();

	void set_current_directory(const string& path);
}