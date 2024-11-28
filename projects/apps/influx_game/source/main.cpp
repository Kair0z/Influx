#include "influx_engine.h"

using namespace influx;

class scene_layer : public engine::layer
{
public:
	virtual void on_start() override
	{
		engine::gameobject* object = layer::create();
		m_transform = object->add_component<engine::transform_component>();
		m_transform->get_transform() = math::transform3D::identity();
		m_transform->get_transform().set_position({});

		engine::sprite_component*  sprite = object->add_component<engine::sprite_component>();
		sprite->set_texture_path("D:/Git/Influx/assets/engine/textures/lego.png");

		engine::mesh_component* mesh = object->add_component<engine::mesh_component>();
		mesh->set_mesh_path("sphere");
	}

	virtual void on_update(const engine::update_context& ctx) override
	{
		const float time = ctx.m_frametime.get_time_seconds();
		const float pos_y = math::pingpong(time, -10.0f, 10.0f);
		m_transform->get_transform().set_position_y(pos_y);
		m_transform->get_transform().update_matrix();
	}

private:
	engine::transform_component* m_transform;
};

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
		create_rootlayer<scene_layer>();
	}
};
influx_engine_game(game);