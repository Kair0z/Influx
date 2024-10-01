#pragma once

#if _DLL
	#define INFLUX_INPUT_API __declspec(dllexport)
#else
	#define INFLUX_INPUT_API __declspec(dllimport)
#endif

#include "core/platform/platform.h"
#include "core/platform/window.h"
#include "core/function.h"

namespace influx::input
{
	struct init_args final
	{
	};

	enum class e_key
	{
		left,
		right,
		down,
		up,
		lshift,
		rshift,
		lctrl,
		rctrl,
		space,
		ascii_num,
		ascii_char,
		count
	};

	struct key_event
	{
		enum class e_type : uint8
		{
			keyup,
			keydown,
			keyhold,
			count
		};

		e_type m_type;
		e_key m_key;
		char m_ascii_char;

		INFLUX_INPUT_API string to_string() const;
	};

	struct mouse_event
	{
		enum class e_type : uint8
		{
			scroll,
			move,
			leave,
			button_down,
			button_up,
			count
		};

		enum class e_button : uint8
		{
			left,
			right,
			middle,
			x,
			count
		};

		e_type m_type;
		e_button m_button;
		float m_wheel_delta;
		math::vectorf2 m_position_client; // relative to window
		math::vectorf2 m_position_screen; // relative to screen
	};

	// allocate internal memory
	INFLUX_INPUT_API void init(const init_args& args = {});

	// platform code should push window events into this function
	// calling service() will translate them into input events
	INFLUX_INPUT_API void push_window_event(const platform::window_event& platform_ev);

	// keyboard inputs
	using key_callback = function<void(const key_event& ev)>;
	INFLUX_INPUT_API void subscribe(const key_callback&);

	// mouse inputs
	using mouse_callback = function<void(const mouse_event& ev)>;
	INFLUX_INPUT_API void subscribe(const mouse_callback&);

	// convenience helper functions
	INFLUX_INPUT_API void subscribe_keydown(const function<void(e_key)>& keydown_callback);
	INFLUX_INPUT_API void subscribe_asciidown(const function<void(char)>& keydown_callback);

	// call this function from any thread to pump the input queue
	INFLUX_INPUT_API void service();

	// cleanup all memory
	INFLUX_INPUT_API void cleanup();
}