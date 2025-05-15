#include "engine_pch.h"
#include "input_state.h"

namespace influx::engine
{
	inline uint32 key_to_index(const input::e_key key)
	{
		influx_assert(key != input::e_key::ascii_char);
		influx_assert(key != input::e_key::ascii_num);

		return static_cast<uint32>(key);
	}

	inline uint32 key_to_index(const char ascii)
	{
		uint32 base = input::k_num_non_ascii_keys;
		return base + static_cast<uint32>(ascii + 127);
	}

	inline uint32 button_to_index(const input::e_mouse_button button)
	{
		return static_cast<uint32>(button);
	}

	inline input::e_key index_to_key(const uint32 index)
	{
		return static_cast<input::e_key>(index);
	}

	void input_state::tick(float delta_seconds)
	{
		m_frame++;

		for (uint32 i = 0u; i < input::k_num_keys; ++i)
		{
			m_keyboard_data.m_keystates[i].m_num_frames++;
		}

		for (uint32 i = 0u; i < input::k_num_mousebuttons; ++i)
		{
			m_mouse_data.m_buttonstates[i].m_num_frames++;
		}
	}

	void input_state::on_keyevent(const input::key_event& ev)
	{
		buttonstate& state = get_keystate(ev);
		switch (ev.m_type)
		{
		case input::key_event::e_type::keydown:
			state.m_is_down = true;
			state.m_num_frames = 0u;
			break;

		case input::key_event::e_type::keyup:
			state.m_is_down = false;
			state.m_num_frames = 0u;
			break;
		}
	}
	void input_state::on_mousevent(const input::mouse_event& ev)
	{
		m_mouse_data.m_prev_mouse_position = m_mouse_data.m_mouse_position;
		m_mouse_data.m_mouse_position = ev.m_position;

		if (ev.m_button != input::e_mouse_button::count)
		{
			buttonstate& state = get_mousebutton_state(ev.m_button);
			switch (ev.m_type)
			{
			case input::mouse_event::type::button_down:
				state.m_is_down = true;
				state.m_num_frames = 0u;
				break;
			case input::mouse_event::type::button_up:
				state.m_is_down = false;
				state.m_num_frames = 0u;
				break;
			}
		}
	}

	const input::mouse_position& input_state::get_mouse_position() const
	{
		return m_mouse_data.m_mouse_position;
	}

	const math::uint2& input_state::get_mouse_position_client() const
	{
		return m_mouse_data.m_mouse_position.m_client;
	}
	const math::uint2& input_state::get_mouse_position_screen() const
	{
		return m_mouse_data.m_mouse_position.m_screen;
	}
	math::vectorf2 input_state::get_mouse_delta_pixels() const
	{
		return m_mouse_data.get_mouse_delta_pixels();
	}

	const buttonstate& input_state::get_keystate(const input::key_event& ev) const
	{ 
		return ev.is_ascii() ? get_keystate(ev.m_ascii_char) : get_keystate(ev.m_key);
	}

	const buttonstate& input_state::get_keystate(input::e_key key) const
	{
		return m_keyboard_data.m_keystates[key_to_index(key)];
	}

	const buttonstate& input_state::get_keystate(char ascii) const
	{
		return m_keyboard_data.m_keystates[key_to_index(ascii)];
	}

	const buttonstate& input_state::get_mousebutton_state(input::e_mouse_button button) const
	{
		return m_mouse_data.m_buttonstates[button_to_index(button)];
	}

	bool input_state::is_down(input::e_key key, uint32* out_num_frames) const
	{
		const buttonstate& state = get_keystate(key);
		if (out_num_frames) *out_num_frames = state.m_num_frames;
		return state.m_is_down;
	}

	bool input_state::is_down(char ascii, uint32* out_num_frames) const
	{
		const buttonstate& state = get_keystate(ascii);
		if (out_num_frames) *out_num_frames = state.m_num_frames;
		return state.m_is_down;
	}

	bool input_state::is_down(input::e_mouse_button button, uint32* out_num_frames) const
	{
		const buttonstate& state = get_mousebutton_state(button);
		if (out_num_frames) *out_num_frames = state.m_num_frames;
		return state.m_is_down;
	}

	buttonstate& input_state::get_keystate(const input::key_event& ev)
	{
		return ev.is_ascii() ? get_keystate(ev.m_ascii_char) : get_keystate(ev.m_key);
	}
	buttonstate& input_state::get_keystate(input::e_key key)
	{
		return m_keyboard_data.m_keystates[key_to_index(key)];
	}
	buttonstate& input_state::get_keystate(char ascii)
	{
		return m_keyboard_data.m_keystates[key_to_index(ascii)];
	}
	buttonstate& input_state::get_mousebutton_state(input::e_mouse_button button)
	{
		return m_mouse_data.m_buttonstates[button_to_index(button)];
	}
}