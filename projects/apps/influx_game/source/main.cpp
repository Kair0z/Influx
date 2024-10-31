#include "influx_engine.h"

class game final : public influx::engine::game_module
{
public:
	virtual void on_config(influx::engine::app_config& app, influx::engine::game_config& game) override
	{
		app
			.set_window_dim({ 640u, 480u });

		game
			.set_gamefile("")
			.set_name("influx_game");
	}

	virtual void on_start() override
	{

	}

	virtual void on_update(const ctx_update& ctx) override
	{
		
	}

	virtual void on_cleanup() override
	{

	}
};
influx_engine_game(game);