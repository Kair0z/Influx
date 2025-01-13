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

		const float camera_distance = 5.0f;
		entity camera = create_entity();
		{
			transform_component& trans_comp = world.create_component<transform_component>(camera);
			{
				trans_comp.set_position({ 0, camera_distance, camera_distance });
				trans_comp.look_at({});
			}

			camera_component& cam_comp = world.create_component<camera_component>(camera);
			{
				cam_comp.set_aspect_ratio(window.get_aspect_ratio());
				cam_comp.set_fov(90.0f);
				cam_comp.set_farplane(1000.0f);
				cam_comp.set_nearplane(0.001f);
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
				mat_comp.set_texture(e_texture_semantic::basecolor, "");
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
