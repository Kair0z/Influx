#include "influx_engine.h"

class actor final
{
public:

};

class game final : public influx::engine::game_module
{
public:
	virtual void on_config(config& config) override
	{
		config
			.set_gamefile("")
			.set_name("influx_game")
			.set_window_dim({640u, 480u});
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