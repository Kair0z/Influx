#pragma once

#include "core/container/vector.h"
#include "core/math/transform.h"
#include "core/math/vector.h"
#include "core/scene/camera.h"

#include "core/platform/window.h"

namespace influx
{
	namespace renderer
	{
		struct scene;
	}
	
	namespace assets
	{
		class flx_scene;
	}

	namespace input
	{
		struct key_event;
	}
}

namespace influx::application
{
	class scene final
	{
		struct mesh_actor final
		{
			string m_mesh_name;
			math::transform3D m_transform;
		};

		struct camera_actor final
		{
			influx::scene::camera m_camera{};
			math::transform3D m_transform{};
		};

		vector<mesh_actor> m_meshes{};
		vector<camera_actor> m_cameras{};

		uint32 m_current_mesh_idx;
		uint32 m_current_cam_idx;

	public:
		scene();

		void on_keyhold(const input::key_event& ev);
		void on_keydown(const input::key_event& ev);
		void on_keyup(const input::key_event& ev);
		void on_mouse_scroll(const float value);

		void update(const frame_time& time);
		const influx::renderer::scene& get_render_scene();

		void save(const std::string& filepath);
		void load(const std::string& filepath);

		void add(const mesh_actor& mesh);
		void add(const camera_actor& camera);

		void set_idx_camera(uint32 new_idx);
		void set_idx_mesh(uint32 new_idx);
		void step_idx_camera(uint32 inc = 1u);
		void step_idx_mesh(uint32 inc = 1u);

	private:
		influx::renderer::scene* mp_render_scene;
		assets::flx_scene* mp_asset_scene;

		struct camera_controls final
		{
			constexpr static float k_max_speed = 1000.0f;
			constexpr static float k_min_speed = 10.0f;

			float m_speed = k_min_speed;
			math::vectorf3 m_velocity;
			math::vectorf3 m_acceleration;
			math::vectorf2 m_input;
		};
		camera_controls m_cam_controls{};

		camera_actor& get_current_camera();
		void reset_camera();

		uint64 m_frame = 0u;
	};
}