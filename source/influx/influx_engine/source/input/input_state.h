#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/container/array.h"

// influx::input
#include "influx_input.h"

namespace influx::engine
{
	struct buttonstate final
	{
		uint8 m_num_frames = 0u;
		bool m_is_down = false;

		inline bool is_down_for_frames(uint8 num_frames = 1) const
		{
			return m_is_down && m_num_frames >= num_frames;
		}
		inline bool is_up_for_frames(uint8 num_frames = 1) const
		{
			return !m_is_down && m_num_frames >= num_frames;
		}
		inline bool is_firstframe_up(uint8 num_frames = 2u) const
		{ 
			return !m_is_down && m_num_frames < num_frames;
		}
		inline bool is_firstframe_down(uint8 num_frames = 2u) const
		{
			return m_is_down && m_num_frames < num_frames;
		}
	};

	struct input_state final
	{
		stat_array<buttonstate, input::k_num_keys>			m_keystates{};
		stat_array<buttonstate, input::k_num_mousebuttons>	m_buttonstates{};
		input::mouse_position								m_mouse_position;
		input::mouse_position								m_prev_mouse_position;

	public:
		uint64 m_frame = 0u;

		const input::mouse_position& get_mouse_position() const;
		const math::uint2& get_mouse_position_client() const;
		const math::uint2& get_mouse_position_screen() const;
		math::vectorf2 get_mouse_delta_pixels() const;
		const buttonstate& get_keystate(const input::key_event& ev) const;
		const buttonstate& get_keystate(input::e_key key) const;
		const buttonstate& get_keystate(char ascii) const;
		const buttonstate& get_mousebutton_state(input::e_mouse_button) const;

		bool is_down(input::e_key key,	uint32* out_num_frames = nullptr) const;
		bool is_down(char ascii,		uint32* out_num_frames = nullptr) const;
		bool is_down(input::e_mouse_button, uint32* out_num_frames = nullptr) const;
		void tick(float delta_seconds);

	public:
		void on_keyevent(const input::key_event& ev);
		void on_mousevent(const input::mouse_event& ev);

	private:
		buttonstate& get_keystate(const input::key_event& ev);
		buttonstate& get_keystate(input::e_key key);
		buttonstate& get_keystate(char ascii);
		buttonstate& get_mousebutton_state(input::e_mouse_button);
	};
}