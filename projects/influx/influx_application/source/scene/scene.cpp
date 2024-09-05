#include "app_pch.h"
#include "scene.h"
#include "application/application_backend.h"

#include "core/log.h"
#include "core/math/math.h"

// renderer
#include "influx_renderer/scene.h"

// assets
#include "influx_assets.h"

// input
#include "influx_input.h"

// imgui
#include "editor/editor.h"
#include "imgui/imgui.h"

namespace influx::application
{
	scene::scene()
		: m_current_cam_idx{}
		, m_current_mesh_idx{}
	{
		mp_render_scene = new influx::renderer::scene();
		mp_asset_scene = new assets::flx_scene();

		math::vectorf3 scene_center = math::vectorf3::zero();

		// camera
		camera_actor cam{};
		cam.m_camera.set_farplane(1000.0f);
		cam.m_camera.set_nearplane(0.0001f);
		cam.m_camera.set_fov(90.0f);
		cam.m_transform = math::transform3D::identity();
		add(cam);

		// meshes
		float distance = 500.0f;
		for (size_t i = 0u; i < 4u; ++i)
		{
			mesh_actor mesh{};
			mesh.m_mesh_name = "transistor1";
			mesh.m_transform = math::transform3D::identity();
			mesh.m_transform.set_position(distance * random::get_random_unit_vectorf3());
			mesh.m_transform.set_position_y(0.0f);
			mesh.m_transform.set_position_z(-10.0f);
			mesh.m_transform.set_scale(0.05f);
			mesh.m_transform.update_matrix();
			add(mesh);
		}

		mesh_actor ground{};
		ground.m_mesh_name = "engine_plane";
		ground.m_transform = math::transform3D::identity();
		ground.m_transform.set_scale(100.0f);
		ground.m_transform.update_matrix();
		add(ground);

		mesh_actor box{};
		box.m_mesh_name = "box";
		add(box);

		// subscribe to input events
		input::subscribe([this](const input::key_event& ev)
		{
			switch (ev.m_type)
			{
			case input::key_event::e_type::keydown:
				on_keydown(ev); 
				break;

			case input::key_event::e_type::keyup: 
				on_keyup(ev);
				break;

			case input::key_event::e_type::keyhold: 
				on_keyhold(ev);
				break;
			}
		});
		input::subscribe([this](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::e_type::button_down:
				on_mouse_button_down(ev.m_button);
				break;

			case input::mouse_event::e_type::button_up:
				on_mouse_button_up(ev.m_button);
				break;

			case input::mouse_event::e_type::move:
				on_mouse_move(ev.m_position_client);
				break;

			case input::mouse_event::e_type::scroll:
				on_mouse_scroll(ev.m_wheel_delta);
				break;
			}
		});

		// imgui
		application::get_editor()->subscribe([this]()
		{
			camera_actor& cam = get_current_camera();

			ImGui::Begin("Scene");

			// draw the cameras
			ImGui::Text("Camera:");
			editor::draw_transform(cam.m_transform, "Transform");
			editor::draw_camera(cam.m_camera);

			ImGui::End();
		});
	}

	void apply_ascii(math::vectorf3& input, char ascii, float value)
	{
		switch (ascii)
		{
		case 'A': input.x += -value; break;
		case 'W': input.z += value; break;
		case 'D': input.x += value; break;
		case 'S': input.z += -value; break;
		}

		input.x = math::clamp(input.x, -1.0f, 1.0f);
		input.z = math::clamp(input.z, -1.0f, 1.0f);
	}

	void scene::on_keyhold(const input::key_event& ev)
	{

	}

	void scene::on_keydown(const input::key_event& ev)
	{
		apply_ascii(m_cam_controls.m_input, ev.m_ascii_char, 1.0f);

		// reset camera on 'R'
		if (ev.m_ascii_char == 'R')
		{
			reset_camera();
		}

		if (ev.m_key == input::e_key::lctrl)
		{
			m_cam_controls.m_input.y = -1.0f;
		}

		if (ev.m_key == input::e_key::space)
		{
			m_cam_controls.m_input.y = 1.0f;
		}
	}

	void scene::on_keyup(const input::key_event& ev)
	{
		apply_ascii(m_cam_controls.m_input, ev.m_ascii_char, -1.0f);

		if (ev.m_key == input::e_key::lctrl)
		{
			m_cam_controls.m_input.y = 0.0f;
		}

		if (ev.m_key == input::e_key::space)
		{
			m_cam_controls.m_input.y = 0.0f;
		}
	}

	void scene::on_mouse_button_down(const input::mouse_event::e_button& button)
	{
		switch (button)
		{
		case input::mouse_event::e_button::right:
			m_cam_controls.m_is_orienting = true;
			m_cam_controls.m_first_frame_orient = true;
			break;
		}
	}

	void scene::on_mouse_button_up(const input::mouse_event::e_button& button)
	{
		switch (button)
		{
		case input::mouse_event::e_button::right:
			m_cam_controls.m_is_orienting = false;
			m_cam_controls.m_first_frame_orient = false;
			break;
		}
	}

	void scene::on_mouse_move(const math::vectorf2& window_pos)
	{
		if (m_cam_controls.m_is_orienting)
		{
			// on the first frame of orienting, set previous to the current
			if (m_cam_controls.m_first_frame_orient)
			{
				m_cam_controls.m_previous_mouse_windowpos = window_pos;
				m_cam_controls.m_first_frame_orient = false;
			}
			
			// set current mouse window position
			m_cam_controls.m_mouse_windowpos = window_pos;
		}
	}

	void scene::on_mouse_scroll(const float value)
	{
		m_cam_controls.m_speed += value;
		m_cam_controls.m_speed = math::clamp(m_cam_controls.m_speed, 
			camera_controls::k_min_speed, 
			camera_controls::k_max_speed * m_cam_controls.m_shift_multiplier);
	}

	void scene::update(const frame_time& time)
	{
		camera_actor& cam = get_current_camera();

		if (m_frame == 0u)
		{
			reset_camera();
		}

		update_camera_move(time);
		update_camera_orient(time);

		// update render scene
		mp_render_scene->m_delta_seconds = time.m_delta_seconds;
		mp_render_scene->m_seconds = time.m_time_seconds;

		influx::renderer::camera& render_cam = mp_render_scene->m_camera;
		render_cam.m_transform = cam.m_transform;
		render_cam.m_far_plane = cam.m_camera.get_farplane();
		render_cam.m_near_plane = cam.m_camera.get_nearplane();
		render_cam.m_fov = cam.m_camera.get_fov();

		// increment framecount
		++m_frame;
	}

	void scene::update_camera_move(const frame_time& time)
	{
		camera_actor& cam = get_current_camera();

		// update velocity:
		float acc_power = 1000.0f;
		m_cam_controls.m_acceleration.x = m_cam_controls.m_input.x * acc_power;
		m_cam_controls.m_acceleration.y = m_cam_controls.m_input.y * acc_power;
		m_cam_controls.m_acceleration.z = m_cam_controls.m_input.z * acc_power;

		m_cam_controls.m_velocity += m_cam_controls.m_acceleration * time.m_delta_seconds;
		m_cam_controls.m_velocity.clamp_length(m_cam_controls.m_speed);

		// move horizontal (local)
		math::vectorf3 vel_horizontal = m_cam_controls.m_velocity;
		vel_horizontal.y = 0.0f;
		cam.m_transform.translate(vel_horizontal * time.m_delta_seconds, true);

		// move vertical (world)
		math::vectorf3 vel_vertical = m_cam_controls.m_velocity;
		vel_vertical.x = vel_vertical.z = 0.0f;
		cam.m_transform.translate(vel_vertical * time.m_delta_seconds, false);

		// drag
		if (m_cam_controls.m_input.is_zero())
		{
			m_cam_controls.m_velocity.lerp_towards(math::vectorf3::zero(), time.m_delta_seconds * acc_power);
		}

		// update matrix
		cam.m_transform.update_matrix();
	}

	void scene::update_camera_orient(const frame_time& time)
	{
		camera_actor& cam = get_current_camera();

		if (m_cam_controls.m_is_orienting)
		{
			// update delta mousepos
			math::vectorf2 delta_mouse = m_cam_controls.m_mouse_windowpos - m_cam_controls.m_previous_mouse_windowpos;
			m_cam_controls.m_previous_mouse_windowpos = m_cam_controls.m_mouse_windowpos;
			
			//  orient
			if (!delta_mouse.is_zero())
			{
				delta_mouse.normalize();
				delta_mouse.y = -delta_mouse.y;
				delta_mouse.x = -delta_mouse.x;

				cam.m_transform.rotate_y(delta_mouse.x * time.m_delta_seconds);
				cam.m_transform.rotate_x(delta_mouse.y * time.m_delta_seconds);
				cam.m_transform.update_matrix();
			}
		}
	}

	void scene::save(const string& filepath)
	{
		return;
		// save to file
		mp_asset_scene->save(filepath);
	}

	void scene::load(const string& filepath)
	{
		return;
		// load the asset scene from file
		mp_asset_scene->load(filepath);
	}

	void scene::add(const camera_actor& camera)
	{
		m_cameras.push_back(camera);
	}

	void scene::add(const mesh_actor& mesh)
	{
		m_meshes.push_back(mesh);

		// invert position
		math::transform3D inverse_transform = mesh.m_transform;
		inverse_transform.set_position_z(-mesh.m_transform.get_position().z);

		influx::renderer::mesh_instance mesh_instance{};
		mesh_instance.m_name = mesh.m_mesh_name;
		mesh_instance.m_transform = inverse_transform.get_matrix();
		mesh_instance.m_material_name = "mat_transistor";
		mp_render_scene->m_meshes.push_back(mesh_instance);
	}

	void scene::set_idx_camera(uint32 new_idx)
	{
		m_current_cam_idx = new_idx % m_cameras.size();
	}

	void scene::set_idx_mesh(uint32 new_idx)
	{
		m_current_mesh_idx = new_idx % m_meshes.size();
	}

	void scene::step_idx_camera(uint32 inc)
	{
		set_idx_camera(m_current_cam_idx + inc);
	}

	void scene::step_idx_mesh(uint32 inc)
	{
		set_idx_mesh(m_current_mesh_idx + inc);
	}

	scene::camera_actor& scene::get_current_camera()
	{
		return m_cameras[m_current_cam_idx];
	}

	void scene::reset_camera()
	{
		camera_actor& cam = get_current_camera();
		cam.m_transform.set_position(0.0f, 2.0f, 10.0f);
		cam.m_transform.look_at(math::vectorf3::zero());
		cam.m_transform.update_matrix();

		m_cam_controls.m_acceleration = {};
		m_cam_controls.m_input = {};
		m_cam_controls.m_velocity = {};
	}

	const influx::renderer::scene& scene::get_render_scene()
	{
		return *mp_render_scene;
	}
}