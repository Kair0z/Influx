#pragma once

#if _DLL
	#define INFLUX_INPUT_API __declspec(dllexport)
#else
	#define INFLUX_INPUT_API __declspec(dllimport)
#endif

// influx::core
#include "core/function.h"
#include "core/string.h"
#include "core/math/vector.h"

namespace influx::input
{
	// event types
	struct key_event;
	enum class e_key : uint8;
	struct mouse_event;

	// 1. Initialize memory first
	struct init_args final
	{
	};
	INFLUX_INPUT_API void init(const init_args& args = {});

	// 2. subscribe to events
	using key_callback = function<void(const key_event& ev)>;
	INFLUX_INPUT_API void subscribe(const key_callback&);

	using mouse_callback = function<void(const mouse_event& ev)>;
	INFLUX_INPUT_API void subscribe(const mouse_callback&);

	// 3. call this function from any thread to pump the input queue
	struct service_args final
	{
		uint32 m_max_events_to_service;
	};
	INFLUX_INPUT_API void service(const service_args& args = {});

	// 3. cleanup all memory
	INFLUX_INPUT_API void cleanup();

	// event subscribing
	INFLUX_INPUT_API void subscribe_keydown(const function<void(e_key)>& keydown_callback);
	INFLUX_INPUT_API void subscribe_asciidown(const function<void(char)>& keydown_callback);

	// manually push events into the queue
	INFLUX_INPUT_API void push_external_event(const key_event& ev);
	INFLUX_INPUT_API void push_external_event(const mouse_event& ev);

	// parse input::events from platform::window event
	// INFLUX_INPUT_API void push_window_event(const platform::window_event& platform_ev);

#pragma region key_events
	enum class e_key : uint8
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
	
	constexpr const char* to_cstr(const e_key key)
	{
		switch (key)
		{
		case e_key::left:		return "left";
		case e_key::right:		return "right";
		case e_key::down:		return "down";
		case e_key::up:			return "up";
		case e_key::lshift:		return "left shift";
		case e_key::rshift:		return "right shift";
		case e_key::lctrl:		return "left ctrl";
		case e_key::rctrl:		return "right ctrl";
		case e_key::space:		return "space";
		case e_key::ascii_num:	return "ascii_num";
		case e_key::ascii_char: return "ascii_char";
		}

		return "unknown";
	}

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
#pragma endregion

#pragma region mouse_events
	struct mouse_event
	{
		enum class e_type : uint8;
		enum class e_button : uint8;

		struct mouse_position final
		{
			mouse_position() = default;
			mouse_position(const math::vectorf2& client, const math::vectorf2& screen)
				: m_client{client}
				, m_screen(screen)
			{}

			math::vectorf2 m_client; // relative to window
			math::vectorf2 m_screen; // relative to screen
		};

		e_type m_type;
		e_button m_button;
		float m_wheel_delta;
		mouse_position m_position;

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
	};
#pragma endregion
}