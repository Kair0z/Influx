#include "app_pch.h"
#include "scene.h"
#include "application/application_backend.h"

#include "core/log.h"

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
		mp_render_scene = new renderer::scene();
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
		mesh.m_mesh_name = "transistor";
		mesh.m_transform = math::transform3D::identity();
		add(mesh);

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
	}

	void apply_ascii(math::vectorf2& input, char ascii, float value)
	{
		switch (ascii)
		{
		case 'A': input.x = -value; break;
		case 'W': input.y = value; break;
		case 'D': input.x = value; break;
		case 'S': input.y = -value; break;
		}
	}
	void scene::on_keyhold(const input::key_event& ev)
	{
		influx::logn("keyhold! [{}]", ev.to_string());
	}

	void scene::on_keydown(const input::key_event& ev)
	{
		influx::logn("keydown! [{}]", ev.to_string());

		apply_ascii(m_cam_controls.m_input, ev.m_ascii_char, 1.0f);

		// reset camera on 'R'
		if (ev.m_ascii_char == 'R')
		{
			reset_camera();
		}
	}

	void scene::on_keyup(const input::key_event& ev)
	{
		influx::logn("keyup! [{}]", ev.to_string());

		apply_ascii(m_cam_controls.m_input, ev.m_ascii_char, 0.0f);
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
		float max_speed = 10.0f;
		m_cam_controls.m_acceleration.x = m_cam_controls.m_input.x * acc_power;
		m_cam_controls.m_acceleration.z = m_cam_controls.m_input.y * acc_power;
		m_cam_controls.m_acceleration.y = 0.0f;

		m_cam_controls.m_velocity += m_cam_controls.m_acceleration * time.m_delta_seconds;
		m_cam_controls.m_velocity.clamp_length(max_speed);

		// update camera
		cam.m_transform.translate(m_cam_controls.m_velocity * time.m_delta_seconds);

		// drag camera
		if (m_cam_controls.m_input.is_zero())
		{
			m_cam_controls.m_velocity.lerp_towards(math::vectorf3::zero(), time.m_delta_seconds * acc_power);
		}
		
		cam.m_transform.update_matrix();

		// update render scene
		renderer::camera& render_cam = mp_render_scene->m_camera;
		render_cam.m_transform = cam.m_transform.get_matrix();
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

		// update the rest
	}

	void scene::add(const camera_actor& camera)
	{
		m_cameras.push_back(camera);
	}

	void scene::add(const mesh_actor& mesh)
	{
		m_meshes.push_back(mesh);

		renderer::mesh_instance mesh_instance{};
		mesh_instance.m_name = mesh.m_mesh_name;
		mesh_instance.m_transform = mesh.m_transform.get_matrix();
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
		cam.m_transform.set_position(0.0f, 0.0f, 10.0f);
		cam.m_transform.look_at(math::vectorf3::zero());

		m_cam_controls.m_acceleration = {};
		m_cam_controls.m_input = {};
		m_cam_controls.m_velocity = {};
	}

	const renderer::scene& scene::get_render_scene()
	{
		return *mp_render_scene;
	}
}