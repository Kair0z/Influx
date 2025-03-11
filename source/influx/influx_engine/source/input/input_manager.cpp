#include "engine_pch.h"
#include "input_manager.h"

#include "editor/editor_manager.h"

namespace influx::engine
{
	static input::mouse_position g_mouse_position{};
	class input_editor final : public editor_window
	{
	public:
		virtual void on_run() override
		{
			ImGui::Text("client: %.2f,%.2f", g_mouse_position.m_client.x, g_mouse_position.m_client.y);
			ImGui::Text("screen: %.2f,%.2f", g_mouse_position.m_screen.x, g_mouse_position.m_screen.y);
			ImGui::Text("screen norm: %.2f,%.2f", g_mouse_position.m_screen_normalized.x, g_mouse_position.m_screen_normalized.y);
			ImGui::Text("client norm: %.2f,%.2f", g_mouse_position.m_client_normalized.x, g_mouse_position.m_client_normalized.y);
		}
	};

	input_manager::input_manager()
	{
		// editor::editor_manager::static_window<input_editor>("input");

		influx::input::init();

		input::subscribe([this](const input::key_event& ev)
		{
			m_state.on_keyevent(ev);
		});

		input::subscribe([this](const input::mouse_event& ev)
		{
			m_state.on_mousevent(ev);

			g_mouse_position = ev.m_position;
		});
	}

	input_manager::~input_manager()
	{
	}

	void input_manager::tick()
	{
		// service input queue
		input::service_args args{};
		args.m_max_events_to_service = 64u;
		input::service(args);

		m_state.tick(0.0f);
	}

	void input_manager::push_window_event(const platform::window_event& ev)
	{
		input::push_window_event(ev);
	}

	const math::vectoru2& input_manager::get_mouse_position_client() const
	{
		return m_state.get_mouse_position_client();
	}
	const math::vectoru2& input_manager::get_mouse_position_screen() const
	{
		return m_state.get_mouse_position_screen();
	}

	math::vectorf2 input_manager::get_mouse_delta() const
	{
		return m_state.get_mouse_delta();
	}

	const buttonstate& input_manager::get_keystate(const input::key_event& ev) const
	{
		return m_state.get_keystate(ev);
	}

	const buttonstate& input_manager::get_keystate(input::e_key key) const
	{
		return m_state.get_keystate(key);
	}

	const buttonstate& input_manager::get_keystate(char ascii) const
	{
		return m_state.get_keystate(ascii);
	}

	const buttonstate& input_manager::get_mousebutton_state(input::e_mouse_button button) const
	{
		return m_state.get_mousebutton_state(button);
	}
	bool input_manager::is_down(input::e_key key, uint32* out_num_frames) const
	{
		return m_state.is_down(key, out_num_frames);
	}
	bool input_manager::is_down(char ascii, uint32* out_num_frames) const
	{
		return m_state.is_down(ascii, out_num_frames);
	}
	bool input_manager::is_down(input::e_mouse_button button, uint32* out_num_frames) const
	{
		return m_state.is_down(button, out_num_frames);
	}
}