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

	void game_module::update(const update_context& ctx)
	{
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