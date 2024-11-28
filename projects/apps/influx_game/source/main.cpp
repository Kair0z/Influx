#include "influx_engine.h"

using namespace influx;

class scene_layer : public engine::layer
{
public:
	virtual void on_start() override
	{
		// mesh
		engine::gameobject* object = layer::create();
		m_transform = object->add_component<engine::transform_component>();
		math::transform3D& transform = m_transform->get_transform();
		transform = math::transform3D::identity();
		transform.set_position({});

		engine::sprite_component*  sprite = object->add_component<engine::sprite_component>();
		sprite->set_texture_path("lego");

		engine::mesh_component* mesh = object->add_component<engine::mesh_component>();
		mesh->set_mesh_path("sphere");

		// camera
		engine::gameobject* camera_object = layer::create();
		engine::camera_component* camera_comp = camera_object->add_component<engine::camera_component>();
		camera_comp->set_fov(90.0f);

		m_cam_transform = camera_object->add_component<engine::transform_component>();
	}

	virtual void on_update(const engine::update_context& ctx) override
	{
		// mesh
		const float time = ctx.m_frametime.get_time_seconds();
		const float speed = 10.0f;
		const float range = 20.0f;
		const float pos_y = math::pingpong(speed * time, -range, range);
		const float size = 0.01f;

		math::transform3D& transform = m_transform->get_transform();
		transform.set_scale(size);
		transform.set_position_y(pos_y);
		transform.update_matrix();

		// camera
		const float angle = time;
		const float distance = 10.0f;
		math::transform3D& cam_transform = m_cam_transform->get_transform();
		cam_transform.set_position_x(math::sinf(angle) * distance);
		cam_transform.set_position_z(math::cosf(angle) * distance);
		cam_transform.look_at({});
		cam_transform.update_matrix();
	}

private:
	engine::transform_component* m_transform;
	engine::transform_component* m_cam_transform;
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