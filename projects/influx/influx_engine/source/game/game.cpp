#include "engine_pch.h"

namespace influx::engine
{
	void game_module::on_config(app_config& app, game_config& game)
	{
		app
			.set_window_dim({ 640u, 480u });
		game
			.set_gamefile("")
			.set_name("none");
	}

	void game_module::on_start()
	{
	}

	void game_module::on_level_loaded()
	{
	}

	void game_module::on_update(const ctx_update& ctx)
	{
	}

	void game_module::on_cleanup()
	{
	}

	void game_module::load_level(level* level)
	{
	}

	void game_module::load_level(const string& levelname)
	{
	}

	level const* game_module::get_current_level() const
	{
		return nullptr;
	}

	game_module::ctx_update::ctx_update(const frame_time& frtime)
		: m_frametime{ frtime }
	{

	}
}