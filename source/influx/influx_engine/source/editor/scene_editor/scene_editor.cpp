#include "engine_pch.h"
#include "scene_editor.h"

// influx::engine
#include "input/input_manager.h"
#include "world/world.h"
#include "scene/scene.h"
#include "component/component.h"
#include "rendering/render_manager.h"

// influx::renderer
#include "influx_renderer/target.h"

// influx::platform
#include "influx_platform/window.h"

namespace influx::engine::editor
{
	static transform_component* g_transform = nullptr;
	static camera_component* g_camera = nullptr;

	void scene_editor::on_edit_place()
	{
		world& world = get_engine()->get_world();
		scene& scene = get_engine()->get_current_scene();
		platform::window& window = get_engine()->get_window();

		// create camera by default
		{
			static math::vectorf3 start_position = { 0,10,10 };
			scene::entity_id id = scene.create_entity();
			camera_component& camera = scene.create_component<camera_component>(id);
			camera.set_priority(1.0f);
			camera.set_aspect_ratio(window.get_aspect_ratio());
			g_camera = &camera;

			transform_component& transform = *scene.get_component<transform_component>(id);
			transform.set_position(start_position);
			transform.look_at({});
			transform.update_matrix();
			g_transform = &transform;

			movement_component& body = scene.create_component<movement_component>(id);
			input_component& input_comp = scene.create_component<input_component>(id);
			{
				static bool locks[6u]{ false, false, false, false, false, false };
				static math::vectorf3 acceleration{};

				static auto update_acceleration = [&body, &transform]()
				{
					// translate the force
					math::float3 transf_acceleration =
					{
						acceleration.x * transform.get_right() +
						acceleration.y * math::vectorf3::up() +
						acceleration.z * transform.get_forward()
					};
					if (!transf_acceleration.is_zero()) transf_acceleration = transf_acceleration.normalized();
					body.set_acceleration(transf_acceleration * 1.0f);
				};

				input_comp.m_on_keydown = [](input::e_key key)
				{
					switch (key)
					{
					case input::e_key::space:   if (!locks[4u]) { acceleration.y += +1.0f;		locks[4u] = true; } break;
					case input::e_key::lshift:  if (!locks[5u]) { acceleration.y += -1.0f;		locks[5u] = true; } break;
					}
					update_acceleration();
				};
				input_comp.m_on_keyup = [](input::e_key key)
				{
					switch (key)
					{
					case input::e_key::space:   if (locks[4u]) { acceleration.y -= +1.0f;		locks[4u] = false; } break;
					case input::e_key::lshift:	if (locks[5u]) { acceleration.y -= -1.0f;		locks[5u] = false; } break;
					}

					update_acceleration();
				};
				input_comp.m_on_ascii_down = [&transform](const char ascii)
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
					case 'R': 
						transform.set_position(start_position); 
						transform.look_at({});
						break;
					}
				};
				input_comp.m_on_ascii_up = [](const char ascii)
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
				input_comp.m_on_mouse_move = [&transform](const input::mouse_position& position)
				{
					math::float2 mousepos_current = position.m_client;
					math::float2 mousepos_delta = mousepos_current - mousepos_prev;
#if 0
					static platform::window& window = get_engine()->get_window();
					const float ar = window.get_aspect_ratio();
					mousepos_delta.y *= ar; // normalize mousemove
#endif

					if (mouse_down && mousepos_delta.is_zero() == false)
					{
						const frame_time& time = get_engine()->get_time();
						const float seconds = time.get_time_seconds();
						const float delta_seconds = time.get_delta_seconds();

						transform.rotate(
							mousepos_delta.y * delta_seconds,
							mousepos_delta.x * delta_seconds,
							0.0f);
					}

					mousepos_prev = mousepos_current;
				};
			}
		}
	}

	void scene_editor::on_edit_remove()
	{
		
	}

	void scene_editor::pick_scene(const input::mouse_position& position) const
	{
		if (g_camera && g_transform)
		{
			world& world = get_engine()->get_world();
			world::trace_result result = world.trace_main_scene(position.m_client_normalized).get();
			if (result.m_is_hit)
			{
				engine::log(e_log_category::info, "trace hit!");
			}
		}
	}

	scene_editor::scene_editor()
	{
		m_edit_radial.set_radius(60.0f);
		m_edit_radial.set_item("place", scene_editor::on_edit_place);
		m_edit_radial.set_item("remove", scene_editor::on_edit_remove);
	}

	scene_editor::~scene_editor()
	{

	}

	void scene_editor::on_imgui(ImGuiContext& ctx)
	{
		input_manager& inputman = get_engine()->get_input();
		m_edit_radial.render(inputman.get_mouse_position_client());

		if (m_edit_radial.has_selection())
		{
			on_radial_select* ptr_ptr = m_edit_radial.get_selected();
			if (ptr_ptr) (*ptr_ptr)();
		}

		math::uint2 view_dimensions = { 640u, 480 };

		render_manager& renderman = get_engine()->get_renderer();
		render_view& scene_view = renderman.get_renderview("scene_view", view_dimensions);
		const renderer::target& target = scene_view.get_target();
		renderer::scene& scene = scene_view.get_scene();
		scene = renderer::scene();

		// just add a gizmo
		renderer::camera camera{};
		math::matrix4x4f cam_transform = math::matrix4x4f::make_transform_RH({ 0,0,10 }, { 0,0,-1 });
		scene.set_camera(camera, cam_transform);
		scene.add_gizmo_transform(math::transform3D::identity());
		
		if (ImGui::Begin("scene"))
		{
			ImGui::Image(reinterpret_cast<ImTextureID>(&target), { (float)view_dimensions.x, (float)view_dimensions.y });
			ImGui::End();
		}
	}

	void scene_editor::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::left: 
			pick_scene(position);
			break;
		case input::e_mouse_button::right:
			m_edit_radial.set_visible(true);
			m_edit_radial.set_position(position.m_client);
			break;
		}
	}

	void scene_editor::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_edit_radial.set_visible(false);
			break;
		}
	}
}