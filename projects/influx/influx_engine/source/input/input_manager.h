#pragma once

#include "input_state.h"

// influx::core
#include "core/container/map.h"
#include "core/container/array.h"

// influx::input
#include "influx_input.h"

namespace influx::engine
{
	// collects influx_input events and dispatches them based on engine layers (editor / game / ...)
	class input_manager final
	{
	public:
		input_manager();
		~input_manager();

		void tick();
		void push_window_event(const platform::window_event&);
		
		const math::vectoru2& get_mouse_position_client() const;
		const math::vectoru2& get_mouse_position_screen() const;

		math::vectorf2 get_mouse_delta() const;
		const buttonstate& get_keystate(const input::key_event& ev) const;
		const buttonstate& get_keystate(input::e_key key) const;
		const buttonstate& get_keystate(char ascii) const;
		const buttonstate& get_mousebutton_state(input::e_mouse_button) const;

		bool is_down(input::e_key key, uint32* out_num_frames = nullptr) const;
		bool is_down(char ascii, uint32* out_num_frames = nullptr) const;
		bool is_down(input::e_mouse_button, uint32* out_num_frames = nullptr) const;

	private:
		input_state m_state{};
	};
}