#pragma once

// influx::core
#include "core/container/map.h"
#include "core/time.h"
#include "core/result.h"

// influx::input
#include "influx_input.h"

// influx::file
#include "influx_file.h"

// influx::imgui
#include "influx_imgui/imgui_widgets.h" // imgui::popup_radial

// influx::engine
#include "editor_window.h"

// imgui
struct ImGuiContext;

namespace influx::engine
{
	class editor_module;
	class engine;

#pragma region helpers
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
			: m_cooldown{ cooldown }
			, m_state{}
			, m_time_last_set{} {}

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
#pragma endregion

	class editor_manager final
	{
	public:
		editor_manager(editor_module* editor);

		result<> update_imgui(ImGuiContext& ctx);

		// sets the editor up to load assets & target the named game
		result<> load_project(engine& engine, const string& name);

		bool has_project() const;
		result<string> get_projectname() const;

		template <typename _t>
		static _t& static_window(const string& tag);

	private:
		editor_module* m_editor = nullptr;
		result<> initialize_inputs();

		compound_keybind_tracker m_keybinds;
		cooldown_toggle m_content_toggle = 0.5f;
		cooldown_toggle m_fps_toggle = 0.5f;
		cooldown_toggle m_engine_toggle = 0.5f;
		cooldown_toggle m_editor_toggle = 0.5f;

		// static windows
		imgui::popup_radial<editor_window*> m_static_windows_radial;
		static umap<string, editor_window*> m_static_windows;

		files::projectfile m_projectfile;

		result<> update_inputs();
		result<> update_context();
		result<> update_mainmenu();
		result<> update_background_dockspace();
		result<> update_static_windows();

		math::vectorf2 m_mousepos;

		result<> on_keydown(input::e_key);
		result<> on_keyup(input::e_key);
		result<> on_ascii_down(char);
		result<> on_ascii_up(char);
		result<> on_mouse_move(const input::mouse_position& position);
		result<> on_mouse_down(input::e_mouse_button button, const input::mouse_position& position);
		result<> on_mouse_up(input::e_mouse_button button, const input::mouse_position& position);
	};

	template<typename _t>
	inline _t& editor_manager::static_window(const string& title)
	{
		static bool first = true;
		static _t static_win = _t{};
		if (first)
		{
			m_static_windows[title] = &static_win;
			first = false;
		}

		return static_win;
	}
}