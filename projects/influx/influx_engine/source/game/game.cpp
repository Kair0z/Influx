#include "engine_pch.h"

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

	void game_module::update(const ctx_update& ctx)
	{
		on_update(ctx);

		// update layer hierarchy
		m_root_layer.on_update();
	}

	void game_module::on_update(const ctx_update& ctx)
	{
	}

	void game_module::on_cleanup()
	{
	}

	game_module::ctx_update::ctx_update(const frame_time& frtime)
		: m_frametime{ frtime }
	{

	}
}