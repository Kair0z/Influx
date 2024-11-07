#pragma once

// influx::core
#include "core/container/map.h"
#include "core/time.h"

// influx::input
#include "influx_input.h"

// influx::engine
#include "file/engine_files.h"

namespace influx::engine
{
	class compound_keybind_tracker final
	{
		enum class e_keystate : uint8
		{
			up,	
			down,
			held,
			count
		};

		umap<input::e_key, e_keystate> m_keys;
		umap<char, e_keystate> m_asciis;

		static constexpr e_keystate determine_new_state(e_keystate old_state, const bool new_state)
		{
			switch (old_state)
			{
			case e_keystate::up: return new_state ? e_keystate::down : e_keystate::up;
			case e_keystate::down: return new_state ? e_keystate::held : e_keystate::up;
			case e_keystate::held: return new_state ? e_keystate::held : e_keystate::up;
			}

			return e_keystate::count;
		}
		static constexpr bool is_down(e_keystate state)
		{
			return state == e_keystate::down || state == e_keystate::held;
		}

	public:
		void set(char ascii, bool state)
		{
			// determine old state
			e_keystate old_state{};
			if (!m_asciis.contains(state))
			{
				old_state = e_keystate::up;
			}
			else
			{
				old_state = m_asciis[state];
			}

			m_asciis[state] = determine_new_state(old_state, state);
		}

		void set(input::e_key key, bool state)
		{
			// determine old state
			e_keystate old_state{};
			if (!m_keys.contains(key))
			{
				old_state = e_keystate::up;
			}
			else
			{
				old_state = m_keys[key];
			}

			m_keys[key] = determine_new_state(old_state, state);
		}

		bool is_dualbind_new(input::e_key a, input::e_key b) const
		{
			if (m_keys.contains(a) && m_keys.contains(b))
			{
				const e_keystate state_a = m_keys.at(a);
				const e_keystate state_b = m_keys.at(b);

				// if both are down and one is new, return true
				if (is_down(state_a) && is_down(state_b))
				{
					return state_a == e_keystate::down || state_b == e_keystate::down;
				}
			}

			return false;
		}
	};

	struct cooldown_toggle final
	{
		cooldown_toggle() = default;
		cooldown_toggle(const float cooldown)
			: m_cooldown{ cooldown } {}

		float m_cooldown;
		bool m_state;
		time::point m_time_last_set;

		void force_set(bool new_state)
		{
			m_state = new_state;
			m_time_last_set = time::get_now();
		}

		void set(bool new_state)
		{
			const float seconds_since_last = time::get_ms_since<float>(m_time_last_set) * 0.001f;
			if (seconds_since_last > m_cooldown)
			{
				force_set(new_state);
			}
		}

		bool operator=(const bool new_state)
		{
			set(new_state);
			return *this;
		}

		operator bool() const
		{
			return m_state;
		}
	};

	class editor_manager final
	{
	public:
		editor_manager(editor_module* editor);

		void on_imgui(ImGuiContext& ctx);

		void on_keydown(input::e_key);
		void on_keyup(input::e_key);
		void on_ascii_down(char);
		void on_ascii_up(char);

		// sets the editor up to load assets & target the named game
		void set_target_game(engine& engine, const string& gamename);

		bool has_game() const;
		string get_game_name() const;

	private:
		editor_module* m_editor = nullptr;
		void initialize_inputs();
		void process_inputs();

		compound_keybind_tracker m_keybinds;
		cooldown_toggle m_content_toggle = 0.5f;
		cooldown_toggle m_engine_toggle = 0.5f;
		cooldown_toggle m_editor_toggle = 0.5f;

		file_game m_current_gamefile;
	};
}