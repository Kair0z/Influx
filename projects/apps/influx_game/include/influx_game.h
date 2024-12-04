#pragma once

#if _DLL
#define INFLUX_GAME_API __declspec(dllexport)
#else
#define INFLUX_GAME_API __declspec(dllimport)
#endif


INFLUX_GAME_API
void foo();

#include "influx_engine/module/game_module.h"

using namespace influx;

class game final : public engine::game_module
{
public:
	virtual void on_config(engine::app_config& app, engine::game_config& game) override
	{
		app.set_window_dim({ 640u, 480u });

		game.set_gameproject_path("influx_game")
			.set_name("influx_game");
	}

	virtual void on_start() override
	{
		
	}
};