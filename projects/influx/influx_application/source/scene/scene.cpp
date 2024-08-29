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
		mesh_actor mesh{};
		mesh.m_mesh_name = "transistor1";
		mesh.m_transform = math::transform3D::identity();
		add(mesh);

		mesh_actor ground{};
		mesh.m_mesh_name = "engine_plane";
		mesh.m_transform = math::transform3D::identity();
		add(ground);

		mesh.m_mesh_name = "box";
		add(mesh);

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
			on_mouse_scroll(ev.m_wheel_delta);
		});
	}

	void apply_ascii(math::vectorf2& input, char ascii, float value)
	{
		switch (ascii)
		{
		case 'A': input.x += -value; break;
		case 'W': input.y += value; break;
		case 'D': input.x += value; break;
		case 'S': input.y += -value; break;
		}

		input.x = math::clamp(input.x, -1.0f, 1.0f);
		input.y = math::clamp(input.y, -1.0f, 1.0f);
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
	}

	void scene::on_keyup(const input::key_event& ev)
	{
		apply_ascii(m_cam_controls.m_input, ev.m_ascii_char, -1.0f);
	}

	void scene::on_mouse_scroll(const float value)
	{
		m_cam_controls.m_speed += value;
		m_cam_controls.m_speed = math::clamp(m_cam_controls.m_speed, camera_controls::k_min_speed, camera_controls::k_max_speed);
	}

	void scene::update(const frame_time& time)
	{
		camera_actor& cam = get_current_camera();

		if (m_frame == 0u)
		{
			reset_camera();
		}

		// update camera velocity:
		float acc_power = 1000.0f;
		m_cam_controls.m_acceleration.x = m_cam_controls.m_input.x * acc_power;
		m_cam_controls.m_acceleration.z = -m_cam_controls.m_input.y * acc_power;
		m_cam_controls.m_acceleration.y = 0.0f;

		m_cam_controls.m_velocity += m_cam_controls.m_acceleration * time.m_delta_seconds;
		m_cam_controls.m_velocity.clamp_length(m_cam_controls.m_speed);

		// update camera
		cam.m_transform.translate(m_cam_controls.m_velocity * time.m_delta_seconds);

		// drag camera
		if (m_cam_controls.m_input.is_zero())
		{
			m_cam_controls.m_velocity.lerp_towards(math::vectorf3::zero(), time.m_delta_seconds * acc_power);
		}
		
		cam.m_transform.update_matrix();

		// update render scene
		influx::renderer::camera& render_cam = mp_render_scene->m_camera;
		render_cam.m_transform = cam.m_transform;
		render_cam.m_far_plane = cam.m_camera.get_farplane();
		render_cam.m_near_plane = cam.m_camera.get_nearplane();
		render_cam.m_fov = cam.m_camera.get_fov();

		++m_frame;
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
		cam.m_transform.set_position(0.0f, 0.0f, 20.0f);
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