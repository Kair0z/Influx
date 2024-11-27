#include "influx_engine.h"

using namespace influx;

class scene_layer : public engine::layer
{
public:
	virtual void on_start() override
	{
		engine::gameobject* object = nullptr; // layer::spawn();
		engine::transform_component* transform = object->add_component<engine::transform_component>();
		transform->get_transform().set_position({});

		engine::sprite_component*  sprite = object->add_component<engine::sprite_component>();
		sprite->set_texture_path("D:/Git/Influx/assets/engine/textures/lego.png");
	}
};

class game final : public engine::game_module
{
public:
	virtual void on_config(engine::app_config& app, engine::game_config& game) override
	{
		app
			.set_window_dim({ 640u, 480u });
		game
			.set_gameproject_path("influx_game")
			.set_name("influx_game");
	}

	virtual void on_start() override
	{
		create_layer<scene_layer>();
	}
};
influx_engine_game(game);