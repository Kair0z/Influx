#pragma once

// influx::core
#include "core/string.h"
#include "core/math/transform.h"
#include "core/scene/camera.h"
#include "core/function.h"
#include "core/math/bounds.h"
#include "core/geometry/sphere.h"
#include "core/macros.h"
#include "core/material/material.h"
#include "core/container/map.h"
#include "core/scene/light.h"

// influx::input
#include "influx_input.h"

// influx::engine
#include "collision/collision.h"

namespace influx::engine
{
	enum class e_component : uint8
	{
		transform,
		sprite,
		mesh,
		material,
		light,
		camera,
		input,
		rigidbody,
		collider,
		scene,
		count
	};
	constexpr static uint32 k_num_component_types = static_cast<uint32>(e_component::count);

	class component
	{
	public:

	};

	namespace detail
	{
		template <e_component _t>
		class tcomponent : public component
		{
		protected:
			tcomponent() = default;
		};
	}

	class transform_component final 
		: public detail::tcomponent<e_component::transform>
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

		void rotate(float x, float y, float z, bool blocal = true)
		{
			m_transform.rotate(x, y, z, blocal);
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

		void move(const math::vectorf3& delta_position)
		{
			set_position(get_position() + delta_position);
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

		void set_rotation(const math::matrix3x3f& rotation)
		{
			m_transform.set_rotation(rotation);
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

		math::vectorf3 get_back() const
		{
			return -get_forward();
		}

		math::vectorf3 get_right() const
		{
			return m_transform.get_right();
		}

		math::vectorf3 get_left() const
		{
			return -get_right();
		}

		math::vectorf3 get_up() const
		{
			return m_transform.get_up();
		}

		math::vectorf3 get_down() const
		{
			return -get_up();
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

	class sprite_component final 
		: public detail::tcomponent<e_component::sprite>
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

	class mesh_component final 
		: public detail::tcomponent<e_component::mesh>
	{
	public:
		mesh_component() = default;

		void set_mesh_name(const string& path)
		{
			m_mesh_name = path;
		}

		const string& get_mesh_name() const
		{
			return m_mesh_name;
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
		string m_mesh_name;
		bool m_is_visible;

		// -- filled in when asset is loaded
		math::boxf m_mesh_boundbox;
		math::spheref m_mesh_boundsphere;
		float m_normalized_scale;
		friend class world;
	};

	class material_component final 
		: public detail::tcomponent<e_component::material>
	{
	public:
		void set_color(const math::vectorf4& color)
		{
			m_material.set_basecolour(color);
		}

		const math::vectorf4& get_color() const
		{
			return m_material.get_basecolour();
		}

		void set_texture(e_texture_semantic semantic, const string& path)
		{
			material::texture_property properties{};
			properties.m_path = path;
			properties.m_semantic = semantic;
			properties.m_texture_index = 0;
			m_material.add_texture(semantic, properties);
		}

		bool has_texture(e_texture_semantic semantic) const
		{
			return m_material.has_texture(semantic);
		}

		string get_texture_path(e_texture_semantic semantic) const
		{
			return m_material.get_texture_name(semantic);
		}

		const material& get_material() const
		{
			return m_material;
		}

	private:
		material m_material{};
	};

	class light_component final
		: public detail::tcomponent<e_component::light>
	{
	public:
		light_component() = default;

		void set_type(e_light_type type)
		{
			m_light.set_type(type);
		}

		e_light_type get_type() const
		{
			return m_light.get_type();
		}

		void set_colour(const math::colour_rgba& colour)
		{
			m_light.set_colour(colour);
		}

		math::colour_rgba get_colour() const
		{
			return m_light.get_colour();
		}

		void set_inner_angle(float inner_angle)
		{
			m_light.set_inner_angle(inner_angle);
		}

		void set_outer_angle(float outer_angle)
		{
			m_light.set_outer_angle(outer_angle);
		}

		float get_inner_angle() const
		{
			return m_light.get_inner_angle();
		}

		float get_outer_angle() const
		{
			return m_light.get_outer_angle();
		}

		void set_attenuation(float att)
		{
			m_light.set_attenuation(att);
		}

		float get_attenuation() const
		{
			return m_light.get_attenuation();
		}

		const influx::light& get_light() const
		{
			return m_light;
		}

	private:
		influx::light m_light;
	};

	class camera_component final 
		: public detail::tcomponent<e_component::camera>
	{
	public:
		camera_component()
		{
			m_camera.set_fov(90.0f);
			m_camera.set_farplane(1000.0f);
			m_camera.set_nearplane(0.0001f);
		}

		void set_fov(float fov)
		{
			m_camera.set_fov(fov);
		}

		float get_fov() const
		{
			return m_camera.get_fov();
		}

		void set_aspect_ratio(float ar)
		{
			m_camera.set_aspect_ratio(ar);
		}

		float get_aspect_ratio() const
		{
			return m_camera.get_aspect_ratio();
		}

		inline math::matrix4x4f get_projection() const
		{
			return m_camera.get_projection();
		}

		void set_nearplane(float n)
		{
			m_camera.set_nearplane(n);
		}

		void set_farplane(float f)
		{
			m_camera.set_farplane(f);
		}

		float get_nearplane() const
		{
			return m_camera.get_nearplane();
		}

		float get_farplane() const
		{
			return m_camera.get_farplane();
		}

		inline const influx::camera& get_camera() const
		{
			return m_camera;
		}

		inline influx::camera get_camera()
		{
			return m_camera;
		}

		influx_property_readwrite(float, priority);

	private:
		influx::camera m_camera{};
	};

	class input_component final 
		: public detail::tcomponent<e_component::input>
	{
	public:
		function<void(input::e_key)> m_on_keydown = {};
		function<void(input::e_key)> m_on_keyup = {};
		function<void(char)> m_on_ascii_down = {};
		function<void(char)> m_on_ascii_up = {};
		function<void(const input::mouse_position&)> m_on_mouse_move = {};
		function<void(input::e_mouse_button button, const input::mouse_position&)> m_on_mouse_down = {};
		function<void(input::e_mouse_button button, const input::mouse_position&)> m_on_mouse_up = {};
	};

	class movement_component final 
		: public detail::tcomponent<e_component::rigidbody>
	{
	public:
		void add_force(const math::float3& force)
		{
			m_acceleration += force;
			logn("add_force({},{},{})", force.x, force.y, force.z);
		}

		void hard_stop()
		{
			m_velocity = math::float3::zero();
		}

		void set_force_x(const float x)
		{
			m_acceleration.x = x;
		}

		void set_force_y(const float y)
		{
			m_acceleration.y = y;
		}

		void set_force_z(const float z)
		{
			m_acceleration.z = z;
		}

		influx_property_readwrite(math::float3, velocity);
		influx_property_readwrite(math::float3, acceleration);
		influx_property_readwrite(float, drag);
		influx_property_readwrite(float, max_speed);
	};

	class collider_component final 
		: public detail::tcomponent<e_component::collider>
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

	class scene_component final
		: public detail::tcomponent<e_component::scene>
	{
	public:
		influx_property_readwrite(bool, scene_active);
	};
}