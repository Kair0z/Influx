#pragma once

#ifndef __CORE_PLATFORM_H_
#define __CORE_PLATFORM_H_

#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/string.h"
#include "core/function.h"
#include "core/singleton/singleton.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace influx::platform
{
	using process_handle = void*;
	using instance_handle = void*;
	using window_handle = void*;
	using event_handle = void*;

	// [IO]
	typedef void(*WindowEventCallback)();
	typedef void(*WindowEvent_MousePos)(const float x, const float y);
	typedef void(*WindowEvent_MouseButton)(int button, bool isDown);
	typedef void(*WindowEvent_MouseWheel)(const float w_x, const float w_y);
	typedef void(*WindowEvent_Focus)(bool isFocussed);

	struct create_window_args final
	{
		create_window_args() = default;
		create_window_args(const math::vectori2& dimensions, const influx::string& name) 
			: m_width{ dimensions.x }, m_height{ dimensions.y }, m_name{ name } {}

		int m_width;
		int m_height;
		
		influx::string m_name;
	};

	enum class e_windowevent
	{
		activate,
		quit,
		max,
		unknown = max
	};

	enum class e_messagebox : uint8
	{
		info,
		warning,
		error,
		max
	};
	
	enum class e_console_colour : uint16
	{
		Green = 2,
		Red = 4,
		Purple = 5,
		BG_Green = 12,
		BG_Red = 14,
		BG_Purple = 15,
		maximum
	};

	enum class e_window_visibility : uint8
	{
		Minimize,
		ShowDefault,
		Maximize
	};
}

// [INTERFACE DECLARATIONS]
namespace influx::platform
{
	// [MEMORY]
	void* allocate(const uint64 dimension);

	/* Allocates memory for an object of sizeof(_t) */
	template <typename _t>
	_t* allocate();
	
	/* Allocates memory for an object of _t and calls _t() */
	template <typename _t, typename ..._args>
	_t* anew(_args&&... args);

	/* Frees memory for an object of sizeof(_t) */
	template <typename _t>
	void free(_t* address);


	// [APPLICATION]
#pragma region application
	process_handle get_current_process();

	instance_handle get_current_instance();

	window_handle get_current_window();

	bool is_window_valid(window_handle handle);

	void quit();
#pragma endregion

	// [WINDOW]
#pragma region window
	window_handle create_window(const create_window_args& args, bool make_open = true);

	void destroy_window(const window_handle handle);

	/* Returns true if window is as a result visible */
	bool set_window_visible(const window_handle windowHandle, const e_window_visibility command);

	template <typename _t>
	math::rect<_t> get_windowrect_full(const window_handle windowHandle);

	template <typename _t>
	math::rect<_t> get_windowrect_client(const window_handle windowHandle);

	bool is_window_visible(const window_handle windowHandle);
#pragma endregion

	// [MISC]
#pragma region miscelaneous
	template <e_messagebox _t>
	void messagebox(const string& caption, const string& message, const window_handle windowHandle);

	inline void messagebox_error(const string& caption, const string& message, const window_handle windowHandle = get_current_window())
	{
		messagebox<e_messagebox::error>(caption, message, windowHandle);
	}

	inline void messagebox_info(const string& caption, const string& message, const window_handle windowHandle = get_current_window())
	{
		messagebox<e_messagebox::info>(caption, message, windowHandle);
	}

	inline void messagebox_warning(const string& caption, const string& message, const window_handle windowHandle = get_current_window())
	{
		messagebox<e_messagebox::warning>(caption, message, windowHandle);
	}

	template <e_console_colour _C>
	void set_console_colour_attribute();
#pragma endregion

#pragma region files
	string get_current_directory();

	void set_current_directory(const string& path);
#pragma endregion
}

#endif // _CORE_PLATFORM_H_