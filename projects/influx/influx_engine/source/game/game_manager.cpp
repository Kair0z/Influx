#include "engine_pch.h"

// influx::core
#include "core/geometry/circle.h"

// influx::engine
#include "game_manager.h"
#include "engine.h"
#include "editor/editor_manager.h"
#include "world/world.h"
#include "component/component.h"

// influx::platform
#include "influx_platform/library.h"
#include "influx_platform/window.h"

namespace influx::engine
{
	inline platform::library* get_game_library()
	{
		// call game module dll start
		string dll_dir = "E:/Git/Influx/bin/debug-windows-x86_64/influx_game/";
		string dll_path = dll_dir + "influx_game.dll";

		// load dll
		static platform::library* lib = nullptr;
		if (lib == nullptr)
		{
			lib = platform::library::load(dll_path);
		}
		return lib;
	}

	void game_manager::start()
	{
		get_game_library()->call("start");

		world& world = *get_engine()->get_world_ptr();
		platform::window& window = *get_engine()->get_window_ptr();

		// create camera
		const float camera_distance = 5.0f;
		const static float max_speed = 10.0f;
		const static float acceleration_rate = 1 * max_speed;
		const static float damp_rate = 0.01f;
		const static float fov = 90.0f;
		static const math::float3 start_position = { 0, camera_distance, camera_distance };
		entity camera = create_entity();
		{
			transform_component& trans_comp = world.create_component<transform_component>(camera);
			{
				trans_comp.set_position(start_position);
				trans_comp.look_at({});
			}

			camera_component& cam_comp = world.create_component<camera_component>(camera);
			{
				cam_comp.set_aspect_ratio(window.get_aspect_ratio());
				cam_comp.set_fov(fov);
				cam_comp.set_farplane(1000.0f);
				cam_comp.set_nearplane(0.001f);
			}
			
			rigidbody_component& rigid_comp = world.create_component<rigidbody_component>(camera);
			{
				rigid_comp.set_drag(damp_rate);
				rigid_comp.set_max_speed(max_speed);
			}

			input_component& input_comp = world.create_component<input_component>(camera);
			{
				static bool locks[4u]{ false, false, false, false };
				input_comp.m_on_ascii_down = [&rigid_comp, &trans_comp](const char ascii)
				{
					switch (ascii)
					{
					case 'W': if (!locks[0u]) { rigid_comp.add_force(acceleration_rate * trans_comp.get_forward());		locks[0u] = true; } break;
					case 'A': if (!locks[1u]) { rigid_comp.add_force(acceleration_rate * trans_comp.get_left());		locks[1u] = true; } break;
					case 'S': if (!locks[2u]) { rigid_comp.add_force(acceleration_rate * trans_comp.get_back());		locks[2u] = true; } break;
					case 'D': if (!locks[3u]) { rigid_comp.add_force(acceleration_rate * trans_comp.get_right());		locks[3u] = true; } break;
					}

					switch (ascii)
					{
					case 'R': trans_comp.set_position(start_position); break;
					}
				};
				input_comp.m_on_ascii_up = [&rigid_comp, &trans_comp](const char ascii)
				{
					switch (ascii)
					{
					case 'W': if (locks[0u]) { rigid_comp.add_force(acceleration_rate * -trans_comp.get_forward()); locks[0u] = false; } break;
					case 'A': if (locks[1u]) { rigid_comp.add_force(acceleration_rate * -trans_comp.get_left());    locks[1u] = false; } break;
					case 'S': if (locks[2u]) { rigid_comp.add_force(acceleration_rate * -trans_comp.get_back());    locks[2u] = false; } break;
					case 'D': if (locks[3u]) { rigid_comp.add_force(acceleration_rate * -trans_comp.get_right());   locks[3u] = false; } break;
					}
				};

				static bool mouse_down = false;
				static math::float2 mousepos_prev{};
				static math::float2 angular_position{};
				input_comp.m_on_mouse_down = [](input::e_mouse_button button, const input::mouse_position& position)
				{
					switch (button)
					{
					case input::e_mouse_button::left: mouse_down = true; break;
					}
				};
				input_comp.m_on_mouse_up = [](input::e_mouse_button button, const input::mouse_position& position)
				{
					switch (button)
					{
					case input::e_mouse_button::left : mouse_down = false; break;
					}
				};
				input_comp.m_on_mouse_move = [&window, &trans_comp](const input::mouse_position& position)
				{
					const float ar = window.get_aspect_ratio();
					math::float2 mousepos_current = position.m_client;
					math::float2 mousepos_delta = mousepos_current - mousepos_prev;
					mousepos_delta.y *= ar; // normalize mousemove

					if (mouse_down && mousepos_delta.is_zero() == false)
					{	
						const frame_time& time = get_engine()->get_time();
						const float seconds = time.get_time_seconds();
						const float delta_seconds = time.get_delta_seconds();

						trans_comp.rotate(
							mousepos_delta.y * delta_seconds,
							mousepos_delta.x * delta_seconds,
							0.0f);
					}

					mousepos_prev = mousepos_current;
				};
			}
		}

		// create swoards
		const uint32 num_swords = 50u;
		math::circlef3D circle = math::circlef3D({}, math::vectorf3::up(), 2.0f);
		for (uint32 i = 0u; i < num_swords; ++i)
		{
			entity sword = create_entity();

			const float angle = i * (360u / num_swords);
			transform_component& trans_comp = world.create_component<transform_component>(sword);
			{
				trans_comp.set_position(circle.get_point_at_degrees(angle));
			}

			mesh_component& mesh_comp = world.create_component<mesh_component>(sword);
			{
				mesh_comp.set_mesh_name("transistor_0");
				mesh_comp.set_use_normalized_scale(true);
			}

			material_component& mat_comp = world.create_component<material_component>(sword);
			{
				mat_comp.set_texture(e_texture_semantic::basecolor, "wood_albedo");
			}
		}
	}

	void game_manager::tick()
	{
		get_game_library()->call("tick");
	}

	void game_manager::end()
	{
		get_game_library()->call("end");

		world& world = *get_engine()->get_world_ptr();

		for (const entity& e : m_entities)
		{
			world.destroy_entity(e);
		}
		m_entities.clear();
	}

	entity game_manager::create_entity()
	{
		world& world = *get_engine()->get_world_ptr();
		m_entities.push_back(world.create_entity());
		return m_entities.back();
	}
}
