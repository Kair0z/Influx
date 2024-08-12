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
		ascii_num,
		ascii_char,
		count
	};

	struct key_event
	{
		enum class e_type
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
		float m_wheel_delta;
	};

	INFLUX_INPUT_API void init(const init_args& args = {});

	INFLUX_INPUT_API void push_window_event(const platform::window_event& platform_ev);

	using key_callback = function<void(const key_event& ev)>;
	INFLUX_INPUT_API void subscribe(const key_callback&);

	using mouse_callback = function<void(const mouse_event& ev)>;
	INFLUX_INPUT_API void subscribe(const mouse_callback&);

	INFLUX_INPUT_API void service();

	INFLUX_INPUT_API void cleanup();
}