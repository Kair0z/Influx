#include "influx_engine.h"

#include "core/container/vector.h"

using namespace influx;

class scene_layer : public engine::layer
{
public:
	struct mesh_object
	{
		mesh_object(layer& layer)
		{
			m_object = layer.create();
			m_mesh = m_object->add_component<engine::mesh_component>();
			m_transform = m_object->add_component<engine::transform_component>();
			m_material = m_object->add_component<engine::material_component>();
		}

		engine::gameobject* m_object = nullptr;
		engine::mesh_component* m_mesh = nullptr;
		engine::material_component* m_material = nullptr;
		engine::transform_component* m_transform = nullptr;
	};

	struct camera_object
	{
		camera_object(layer& layer)
		{
			m_object = layer.create();
			m_camera = m_object->add_component<engine::camera_component>();
			m_transform = m_object->add_component<engine::transform_component>();
		}

		engine::gameobject* m_object = nullptr;
		engine::camera_component* m_camera = nullptr;
		engine::transform_component* m_transform = nullptr;
	};

	virtual void on_start() override
	{
		// mesh
		for (uint32 i = 0u; i < 100u; ++i)
		{
			m_meshes.push_back(mesh_object(*this));
			mesh_object& object = m_meshes[i];

			object.m_mesh->set_mesh_path("sphere");
			auto& transform = object.m_transform->get_transform();
			const float padding = 200.0f;
			float x = (i % 10u) * padding;
			float y = (i / 10u) * padding;
			const float offset = -(5 * padding);

			math::vectorf3 position = { x + offset, 0.0f, y + offset };
			transform.set_position(position);
			transform.set_scale(0.01f);
			transform.update_matrix();
		}

		// camera
		m_cameras.push_back(camera_object(*this));
		m_cameras[0u].m_camera->set_fov(120.0f);
	}

	virtual void on_update(const engine::update_context& ctx) override
	{
		const float time = ctx.m_frametime.get_time_seconds();
		const float angle = time;
		const float distance = 10.0f;

		math::transform3D& cam_transform = m_cameras[0u].m_transform->get_transform();
		cam_transform.set_position_x(math::sinf(angle) * distance);
		cam_transform.set_position_z(math::cosf(angle) * distance);
		cam_transform.set_position_y(10.0f);
		cam_transform.look_at({});
		cam_transform.update_matrix();

		const float value = math::pingpong(time, 0.0f, 1.0f);
		const math::vectorf4 colour = { value, value, value, 1.0f};
		for (uint32 i = 0u; i < 100u; ++i)
		{
			mesh_object& object = m_meshes[i];
			object.m_material->set_color(colour);
		}
	}

	virtual void on_keydown(input::e_key) override 
	{
	}

	virtual void on_keyup(input::e_key) override 
	{
	}

	virtual void on_ascii_down(char) override 
	{
	}

	virtual void on_ascii_up(char) override 
	{
	}

	virtual void on_mouse_move(const input::mouse_position& position) override 
	{
	}

	virtual void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position) override 
	{
	}

	virtual void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position) override 
	{
	}

private:
	vector<mesh_object> m_meshes{};
	vector<camera_object> m_cameras{};
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