#pragma once

// influx::core
#include "core/string.h"
#include "core/math/transform.h"
#include "core/scene/camera.h"
#include "core/function.h"
#include "core/math/bounds.h"
#include "core/geometry/sphere.h"
#include "core/macros.h"

// influx::input
#include "influx_input.h"

// influx::engine
#include "collision/collision.h"

namespace influx::engine
{
	class component
	{
	protected:
		component() = default;
	};

	class transform_component final : public component
	{
	public:
		transform_component() = default;
		transform_component(const math::transform3D & transform)
			: m_transform{ transform } {}

		const math::transform3D& get_transform() const
		{
			return m_transform;
		}

		math::transform3D& get_transform()
		{
			return m_transform;
		}

		void translate(const math::vectorf3& add_position, bool blocal = false)
		{
			m_transform.translate(add_position, blocal);
		}

		void rotate(float delta_angle, const math::vectorf3& axis)
		{
			m_transform.rotate(delta_angle, axis);
		}

		void rotate_y(float delta_angle, bool blocal = false)
		{
			m_transform.rotate_y(delta_angle, blocal);
		}

		void rotate_x(float delta_angle, bool blocal = false)
		{
			m_transform.rotate_x(delta_angle, blocal);
		}

		void set_position(const math::vectorf3& position)
		{
			m_transform.set_position(position);
		}

		void set_position(float x, float y, float z)
		{
			m_transform.set_position(x,y,z);
		}

		void set_position_x(float x)
		{
			m_transform.set_position_x(x);
		}

		void set_position_y(float y)
		{
			m_transform.set_position_y(y);
		}

		void set_position_z(float z)
		{
			m_transform.set_position_z(z);
		}

		void set_forward(const math::vectorf3& newForward)
		{
			m_transform.set_forward(newForward);
		}

		void set_right(const math::vectorf3& newRight)
		{
			m_transform.set_right(newRight);
		}

		void set_up(const math::vectorf3& newUp)
		{
			m_transform.set_up(newUp);
		}

		void set_scale(const math::vectorf3& scale)
		{
			m_transform.set_scale(scale);
		}

		void set_scale(const float scale)
		{
			m_transform.set_scale(scale);
		}

		void look_at(const math::vectorf3& location)
		{
			m_transform.look_at(location);
		}

		const math::rotation& get_rotation() const
		{
			return m_transform.get_rotation();
		}

		math::vectorf3 get_position() const
		{
			return m_transform.get_position();
		}

		math::vectorf3 get_forward() const
		{
			return m_transform.get_forward();
		}

		math::vectorf3 get_right() const
		{
			return m_transform.get_right();
		}

		math::vectorf3 get_up() const
		{
			return m_transform.get_up();
		}

		math::vectorf3 get_scale() const
		{
			return m_transform.get_scale();
		}

		math::matrix4x4f get_matrix() const
		{
			return m_transform.get_matrix();
		}

		void update_matrix()
		{
			m_transform.update_matrix();
		}

	private:
		math::transform3D m_transform;
	};

	class sprite_component final : public component
	{
	public:
		sprite_component() = default;

		void set_texture_path(const string& path)
		{
			m_texture_filepath = path;
		}

		const string& get_texture_path() const
		{
			return m_texture_filepath;
		}

	private:
		string m_texture_filepath;

		// -- filed in on asset load
		math::vectoru2 m_texture_dimensions;
		friend class world;
	};

	class mesh_component final : public component
	{
	public:
		mesh_component() = default;

		void set_mesh_path(const string& path)
		{
			m_mesh_filepath = path;
		}

		const string& get_mesh_path() const
		{
			return m_mesh_filepath;
		}

		void set_visible(bool new_vis)
		{
			m_is_visible = new_vis;
		}

		bool is_visible() const
		{
			return m_is_visible;
		}

		influx_property_readwrite(bool, use_normalized_scale);
		influx_property_readwrite(bool, invert_normals);

	private:
		string m_mesh_filepath;
		bool m_is_visible;

		// -- filled in when asset is loaded
		math::boxf m_mesh_boundbox;
		math::spheref m_mesh_boundsphere;
		friend class world;
	};

	class material_component final : public component
	{
	public:
		void set_color(const math::vectorf4& color)
		{
			m_color = color;
		}

		const math::vectorf4& get_color() const
		{
			return m_color;
		}

	private:
		math::vectorf4 m_color;
	};

	class camera_component final : public component
	{
	public:
		void set_fov(float fov)
		{
			m_camera.set_fov(fov);
		}

		float get_fov() const
		{
			return m_camera.get_fov();
		}

	private:
		influx::scene::camera m_camera{};
	};

	class input_component final : public component
	{
	public:
		function<void(input::e_key)>	m_on_keydown = {};
		function<void(input::e_key)>	m_on_keyup = {};
		function<void(char)>			m_on_ascii_down = {};
		function<void(char)>			m_on_ascii_up = {};
		function<void(const input::mouse_position&)> m_on_mouse_move = {};
		function<void(input::e_mouse_button button, const input::mouse_position&)> m_on_mouse_down = {};
		function<void(input::e_mouse_button button, const input::mouse_position&)> m_on_mouse_up = {};
	};

	class rigidbody_component final : public component
	{
	public:
		void add_force(const math::float3& force)
		{
			m_acceleration += force;
		}

		void hard_stop()
		{
			m_velocity = math::float3::zero();
		}

		influx_property_readwrite(math::float3, velocity);
		influx_property_readwrite(math::float3, acceleration);
		influx_property_readwrite(float, drag);
	};

	class collider_component final : public component
	{
	public:
		void grow(const math::float3& position)
		{
			m_bounding_sphere.grow_to(position);
			m_bounding_box.grow_to_contain(position);
		}

		void shrink(const math::float3& position)
		{
			m_bounding_sphere.shrink_to(position);
			m_bounding_box.shrink_to_contain(position);
		}

		void grow(const math::boxf& box)
		{
			grow(box.get_minimum());
			grow(box.get_maximum());
		}

		void shrink(const math::boxf& box)
		{
			shrink(box.get_minimum());
			shrink(box.get_maximum());
		}

	private:
		influx_property_read(math::boxf, bounding_box);
		influx_property_read(math::spheref, bounding_sphere);
		influx_property_readwrite(e_collision_layer, layer);
	};
}