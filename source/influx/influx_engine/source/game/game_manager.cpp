#include "influx_game.h"
#include "engine_pch.h"

// influx::core
#include "core/math/circle.h"

// influx::engine
#include "game_manager.h"
#include "engine.h"
#include "editor/editor_manager.h"
#include "world/world.h"
#include "component/component.h"
#include "content/content_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::file
#include "influx_file.h"

namespace influx::engine
{
	game_manager::game_manager()
	{
		const string path_to_dll = get_engine_directory(engine_directory::binaries).get_full_path() + L"/debug-windows-x86_64/pong/pong.dll";
		m_game_library.load(to_string(path_to_dll)).get();
		m_game_library.call_engine_init();
	}

	void game_manager::start()
	{
		if (m_state == state::idle)
		{
			// hardcoded layer
			setup_camera();
			// setup_swords();
			// setup_cafe();
			setup_unitcube();

			// enable game renderview (and set its dimensions)
			window_manager& windowman = get_engine()->get_windowman();
			render_manager& renderman = get_engine()->get_renderer();
			render_view& render_view = renderman.get_renderview(e_render_view::game);
			platform::window& mainwindow = windowman.get_main_window();
			render_view.set_dimensions(mainwindow.get_dimensions());
			render_view.set_render_enabled(true);

			m_game_library.call_start();
			m_state = state::running;
		}
	}

	void game_manager::tick()
	{
		if (m_state == state::idle)
		{
			return;
		}

		m_game_library.call_tick();
	}

	void game_manager::end()
	{
		if (m_state == state::idle)
		{
			return;
		}

		m_game_library.call_end();

		world& world = get_engine()->get_world();

		// save camera to editor
		if (get_engine()->is_editor())
		{
			const entity& camera = m_entities[0];
			transform_component* cam_transform = world.get_component<transform_component>(camera.get_handle());
			get_engine()->get_editor().get_editorfile().m_camera_transform = cam_transform->get_matrix();
		}
		
		for (const entity& e : m_entities)
		{
			world.destroy_entity(e.get_handle());
		}
		m_entities.clear();

		m_state = state::idle;
	}

	void game_manager::setup_camera()
	{
		platform::window& window = get_engine()->get_window();
		world& world = get_engine()->get_world();

		const float camera_distance = 5.0f;
		const static float max_speed = 10.0f;
		const static float acceleration_rate = 1 * max_speed;
		const static float damp_rate = 0.01f;
		const static float fov = 90.0f;

		static math::float3 start_position = math::float3{1,1,1} * camera_distance;
		static math::matrix4x4f start_rotation = math::matrix4x4f::identity();

#if 0
		if (get_engine()->is_editor() && false)
		{
			editor::editor_manager& editorman = get_engine()->get_editor();
			const math::matrix4x4f& cam_transform = editorman.get_editorfile().m_camera_transform;;
			start_position = cam_transform.get_translation();
			start_rotation = cam_transform.get_rotation_matrix();
		}
#endif

		entity camera = create_entity();
		{
			transform_component& trans_comp = world.create_component<transform_component>(camera.get_handle());
			{
				trans_comp.set_position(start_position);
				trans_comp.look_at(start_position);
			}

			camera_component& cam_comp = world.create_component<camera_component>(camera.get_handle());
			{
				cam_comp.set_aspect_ratio(window.get_aspect_ratio());
				cam_comp.set_fov(fov);
				cam_comp.set_farplane(1000.0f);
				cam_comp.set_nearplane(0.001f);
			}

			movement_component& move_comp = world.create_component<movement_component>(camera.get_handle());
			{
				move_comp.set_drag(damp_rate);
				move_comp.set_max_speed(max_speed);
			}

			light_component& light_comp = world.create_component<light_component>(camera.get_handle());
			{
				light_comp.set_colour(colour::k_white);
				light_comp.set_attenuation(2.0f);
				light_comp.set_type(influx::e_light_type::point);
			}

			input_component& input_comp = world.create_component<input_component>(camera.get_handle());
			{
				static bool locks[6u]{ false, false, false, false, false, false };
				static math::vectorf3 acceleration{};

				static auto update_acceleration = [&move_comp, &trans_comp]()
				{
					// translate the force
					math::float3 transf_acceleration =
					{
						acceleration.x * trans_comp.get_right() +
						acceleration.y * math::vectorf3::make_up() +
						acceleration.z * trans_comp.get_forward()
					};
					if (!transf_acceleration.is_zero()) transf_acceleration = transf_acceleration.normalized();
					move_comp.set_acceleration(transf_acceleration * acceleration_rate);
				};

				input_comp.m_on_keydown = [&move_comp, &trans_comp](input::e_key key)
				{
					switch (key)
					{
					case input::e_key::space:   if (!locks[4u]) { acceleration.y += +1.0f;		locks[4u] = true; } break;
					case input::e_key::lshift:  if (!locks[5u]) { acceleration.y += -1.0f;		locks[5u] = true; } break;
					}
					update_acceleration();
				};
				input_comp.m_on_keyup = [&move_comp, &trans_comp](input::e_key key)
				{
					switch (key)
					{
					case input::e_key::space:   if (locks[4u]) { acceleration.y -= +1.0f;		locks[4u] = false; } break;
					case input::e_key::lshift:	if (locks[5u]) { acceleration.y -= -1.0f;		locks[5u] = false; } break;
					}

					update_acceleration();
				};
				input_comp.m_on_ascii_down = [&move_comp, &trans_comp](const char ascii)
				{
					switch (ascii)
					{
					case 'W': if (!locks[0u]) { acceleration.z += +1.0f;		locks[0u] = true; } break;
					case 'A': if (!locks[1u]) { acceleration.x += -1.0f;		locks[1u] = true; } break;
					case 'S': if (!locks[2u]) { acceleration.z += -1.0f;		locks[2u] = true; } break;
					case 'D': if (!locks[3u]) { acceleration.x += +1.0f;		locks[3u] = true; } break;
					}

					update_acceleration();

					// reset position
					switch (ascii)
					{
					case 'R': trans_comp.set_position(start_position); break;
					}
				};
				input_comp.m_on_ascii_up = [&move_comp, &trans_comp](const char ascii)
				{
					switch (ascii)
					{
					case 'W': if (locks[0u]) { acceleration.z -= +1.0f; locks[0u] = false; } break;
					case 'A': if (locks[1u]) { acceleration.x -= -1.0f; locks[1u] = false; } break;
					case 'S': if (locks[2u]) { acceleration.z -= -1.0f; locks[2u] = false; } break;
					case 'D': if (locks[3u]) { acceleration.x -= +1.0f; locks[3u] = false; } break;
					}

					update_acceleration();
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
					case input::e_mouse_button::left: mouse_down = false; break;
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
	}

	void game_manager::setup_swords()
	{
		world& world = get_engine()->get_world();

		// create swoards
		const uint32 num_swords = 50u;
		math::circlef3D circle = math::circlef3D({}, math::vectorf3::make_up(), 2.0f);
		for (uint32 i = 0u; i < num_swords; ++i)
		{
			entity sword = create_entity();

			const float angle = i * (360.0f / num_swords);
			transform_component& trans_comp = world.create_component<transform_component>(sword.get_handle());
			{
				trans_comp.set_position(circle.get_point_at_degrees(angle));
			}

			mesh_component& mesh_comp = world.create_component<mesh_component>(sword.get_handle());
			{
				assets::mesh_id mesh_id = assets::make_mesh_id("transistor_0");
				mesh_comp.set_mesh_id(mesh_id);
				mesh_comp.set_use_normalized_scale(true);
			}

			material_component& mat_comp = world.create_component<material_component>(sword.get_handle());
			{
				mat_comp.set_texture(e_texture_semantic::basecolor, "T_Sword_Opaque_BC");
			}
		}
	}

	void game_manager::setup_cafe()
	{
		world& world = get_engine()->get_world();

		const string& scene_name = "CafeLeBlanc";
		const asset_manager& assetman = get_engine()->get_assetman();
		const assets::scene_id sc_id = assets::make_scene_id(scene_name);
		const assets::scene_asset* leblanc_asset = assetman.find_asset<assets::e_asset_type::scene>(sc_id);
		if (leblanc_asset == nullptr)
		{
			return;
		}

		const imp::scene_data& scene_data = leblanc_asset->get_resource();
		const uint32 num_meshes = scene_data.get_num_meshes();

		for (uint32 i = 0u; i < num_meshes; ++i)
		{
			const imp::mesh_data& mesh = scene_data.get_mesh(i);
			entity sword = create_entity();

			transform_component& trans_comp = world.create_component<transform_component>(sword.get_handle());
			{
				const float scale_multiplier = 0.01f;
				math::matrix4x4f copy_transform = scene_data.get_transform(mesh) * math::matrix4x4f::make_scale( math::float3{ scale_multiplier , scale_multiplier , scale_multiplier });
				trans_comp.set_matrix(copy_transform);
			}

			mesh_component& mesh_comp = world.create_component<mesh_component>(sword.get_handle());
			{
				const string mesh_name = scene_name + "_" + to_string(i);
				const assets::mesh_id mesh_id = assets::make_mesh_id(mesh_name);
				mesh_comp.set_mesh_id(mesh_id);
			}

			material_component& mat_comp = world.create_component<material_component>(sword.get_handle());
			{
				mat_comp.set_texture(e_texture_semantic::diffuse, i % 2 == 0u ? "wood_albedo" : "wood_albedo");
			}
		}
	}

	void game_manager::setup_unitcube()
	{
		world& world = get_engine()->get_world();
		entity cube = create_entity();

		transform_component& trans_comp = world.create_component<transform_component>(cube.get_handle());
		{
		}

		mesh_component& mesh_comp = world.create_component<mesh_component>(cube.get_handle());
		{
			const string mesh_name = "box_0";
			const assets::mesh_id mesh_id = assets::make_mesh_id(mesh_name);
			mesh_comp.set_mesh_id(mesh_id);
		}

		material_component& mat_comp = world.create_component<material_component>(cube.get_handle());
		{
			mat_comp.set_texture(e_texture_semantic::basecolor, "T_Sword_Opaque_BC");
		}
	}

	entity game_manager::create_entity()
	{
		world& world = get_engine()->get_world();
		m_entities.push_back(world.create_entity());
		return m_entities.back();
	}
	
	game_manager::~game_manager()
	{
		if (m_state == state::running)
		{
			end();
		}
	}
}
