#pragma once

#if _DLL
#define INFLUX_PLATFORM_API __declspec(dllexport)
#else
#define INFLUX_PLATFORM_API __declspec(dllimport)
#endif

// influx::core
#include "core/basetypes.h"
#include "core/string.h"
#include "core/math/vector.h"
#include "core/result.h"

// influx::platform
#include "thread.h"

namespace influx::platform
{
	template <typename _t = unsigned char>
	using result = influx::result<_t, const char*>;

	using window_handle = void*;
	using process_handle = void*;
	using instance_handle = void*;
	using event_handle = void*;
	using thread_handle = void*;

	enum class e_console_colour : uint16
	{
		blue = 1,
		green = 2,
		red = 4,
		yellow = green | red,
		purple = red | blue,
		white = green | red | blue,
		bg_blue = 11,
		bg_green = 12,
		bg_red = 14,
		bg_purple = bg_red | bg_blue,
		count
	};

	struct file_dialog_result final
	{
		bool m_has_selected = false;
		string m_selection = "";
	};

	class INFLUX_PLATFORM_API platform
	{
	public:
		static process_handle get_current_process();

		static instance_handle get_current_instance();

		static void quit();

		static void set_console_colour_attribute(e_console_colour colour);

		static string get_current_directory();

		static void set_current_directory(const string& path);

		static math::vectoru2 get_cursor_screenpos();

		static window_handle find_window_from_cursor();

		static file_dialog_result open_file_dialog(const string& path);
	};
}