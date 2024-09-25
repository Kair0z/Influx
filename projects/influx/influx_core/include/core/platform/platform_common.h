#pragma once

#include "core/function.h" // function

namespace influx::platform
{
	using process_handle = void*;
	
	using instance_handle = void*;
	
	using event_handle = void*;

	using window_handle = void*;

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

	enum class e_windowevent : uint8
	{
		activate,
		quit,
		max,
		unknown = max
	};

	enum class e_window_visibility : uint8
	{
		minimized,
		showed,
		maximized,
		count
	};

	enum class e_messagebox : uint8
	{
		info,
		warning,
		error,
		count
	};

	struct window_event final
	{
		enum class type : uint8
		{
			// input
			keydown,
			keyup,

			// mouse
			wheel,
			mouse_move,
			mouse_leave,
			mouse_down,
			mouse_up,

			// general
			activate,
			quit,
			count
		} m_type;

		enum class mouse_button : uint8
		{
			left,
			right,
			middle,
			x,
			count
		};

		enum class key_type : uint8
		{
			left,
			right,
			up,
			down,
			home,
			end,
			insert,
			deleet,
			f2,
			lshift,
			rshift,
			lctrl,
			rctrl,
			space,
			ascii_num, // ascii number (0-9)
			ascii_ch, // ascii character (A-Z)
			unknown,
			count
		};

		key_type parse_key_type() const;
		char parse_ascii() const;
		float parse_wheel_delta() const;
		math::vectorf2 parse_position_window() const;
		math::vectorf2 parse_position_screen() const;
		mouse_button parse_mouse_button() const;

		uint32 m_mssg;
		uint64 m_wParam;
		uint64 m_lParam;
	};

	// window event callback types
	typedef void(*winev_callback)	();
	typedef void(*winev_mousepos)	(const float x, const float y);
	typedef void(*winev_mousebutton)(int button, bool isDown);
	typedef void(*winev_mousewheel)	(const float w_x, const float w_y);
	typedef void(*winev_focus)		(bool is_focussed);

	using window_proc_callback = function<void(const window_event& e)>;
}