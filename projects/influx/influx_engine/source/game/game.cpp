#include "engine_pch.h"

// influx::input
#include "influx_input.h"

namespace influx::engine
{
	void game_module::on_config(app_config& app, game_config& game)
	{
		app
			.set_window_dim({ 640u, 480u });
		game
			.set_gameproject_path("")
			.set_name("none");
	}

	void game_module::on_start()
	{

	}

	void game_module::update(const update_context& ctx)
	{
		static bool first = true;
		if (first)
		{	
			input::subscribe_keydown([this](input::e_key key){ m_layergraph.on_keydown(key); });
			input::subscribe_keyup([this](input::e_key key){ m_layergraph.on_keyup(key); });
			input::subscribe_asciidown([this](char ascii){ m_layergraph.on_ascii_down(ascii); });
			input::subscribe_asciiup([this](char ascii){ m_layergraph.on_ascii_up(ascii); });
			input::subscribe_mousemove([this](const input::mouse_position& position){ m_layergraph.on_mouse_move(position); });
			input::subscribe_mousedown([this](input::e_mouse_button button, const input::mouse_position& position){ m_layergraph.on_mouse_down(button, position);	});
			input::subscribe_mouseup([this](input::e_mouse_button button, const input::mouse_position& position){ m_layergraph.on_mouse_up(button, position); });
			first = false;
		}

		on_update(ctx);

		m_layergraph.update(ctx);
	}

	void game_module::on_update(const update_context& ctx)
	{
	}

	void game_module::on_cleanup()
	{
	}
}