#pragma once

// influx::renderer
#include "influx_renderer/common.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/material.h"

// imgui
struct ImGuiContext;

namespace influx::renderer
{
	class scene;

	static constexpr object_id k_invalid_id = (uint32)-1;

	struct view_matrices final
	{
		matrix m_transform = matrix::identity();	// transform of the camera
		matrix m_projection;
		matrix m_view;				// inv transform
		matrix m_viewprojection;
		matrix m_inv_viewprojection;
		matrix m_inv_projection;

		INFLUX_RENDER_API void update(const camera& camera);
		INFLUX_RENDER_API void update(const math::matrix4x4f& transform, const camera& camera);
	};

	struct light_instance final
	{
		light m_light;
		transform_id m_transform;
	};

	struct mesh_instance final
	{
		mesh_id				m_mesh_id = k_invalid_id;
		material_id			m_mat_id = k_invalid_id;
		transform_id		m_transform_id = k_invalid_id;
		colour				m_per_instance_colour = {};
	};

	struct line final
	{
		position3D m_points[2]{};
		colour m_colour;
		line(const position3D& start, const position3D& end, const colour& colour)
			: m_points{ start, end }
			, m_colour{ colour } {}
	};

	struct world_constants final
	{
		float m_delta_seconds;
		float m_time_seconds;
	};

	class world final
	{
	public:
		vector<matrix>				m_transforms{};
		vector<mesh_instance>		m_meshes = {};
		vector<light_instance>		m_lights = {};
		vector<line>				m_lines = {};
		world_constants				m_constants;

		INFLUX_RENDER_API
		mesh_instance_id add_mesh_instance(
			const mesh_id& mesh,
			const matrix& transform,
			const material_id& material = get_internal_material_id(e_material::none),
			const colour& colour = {});

		INFLUX_RENDER_API
		light_instance_id add_light(const light& light, const matrix& transform);

		INFLUX_RENDER_API 
		result<matrix> get_transform(const transform_id& id) const;

		INFLUX_RENDER_API 
		const vector<mesh_instance>& get_mesh_instances() const;
		
		INFLUX_RENDER_API 
		const vector<line>& get_lines() const;

		INFLUX_RENDER_API 
		const vector<light_instance>& get_lights() const;

		INFLUX_RENDER_API
		uint32 get_num_lights_total() const;

		INFLUX_RENDER_API 
		uint32 get_num_lights(e_light_type type) const;

		INFLUX_RENDER_API
		const world_constants& get_world_constants() const;

		inline mesh_instance_id add_mesh_instance(
			const string& mesh_name,
			const matrix& transform,
			const material_id& material = get_internal_material_id(e_material::none),
			const colour& colour = {}) {
			return add_mesh_instance(make_mesh_id(mesh_name), transform, material, colour);
		}

	private:
		template <typename _id>
		auto& get_container()
		{
			if constexpr (std::is_same_v<_id, transform_id>) {
				return m_transforms;
			}
			else if constexpr (std::is_same_v<_id, mesh_instance_id>) {
				return m_meshes;
			}
			else if constexpr (std::is_same_v<_id, light>) {
				return m_lights;
			}
			else if constexpr (std::is_same_v<_id, line>) {
				return m_lines;
			}
			else {
				static_assert(!std::is_same_v<_id, _id>, "Unsupported ID type passed to get_container");
			}
		}
		template <typename _id>
		const auto& get_container() const
		{
			if constexpr (std::is_same_v<_id, transform_id>) {
				return m_transforms;
			}
			else if constexpr (std::is_same_v<_id, mesh_instance_id>) {
				return m_meshes;
			}
			else if constexpr (std::is_same_v<_id, light>) {
				return m_lights;
			}
			else if constexpr (std::is_same_v<_id, line>) {
				return m_lines;
			}
			else {
				static_assert(!std::is_same_v<_id, _id>, "Unsupported ID type passed to get_container");
			}
		}
		template <typename _id>
		bool in_range(const _id& id) const
		{
			return id < get_container<_id>().size();
		}

		INFLUX_RENDER_API transform_id add_transform(const matrix& mat);

	public:
		world() = default; // create using renderer...
	};

	class worldview final
	{
	public:
		camera					m_camera_settings;
		view_matrices			m_matrices;
		world*					m_world;

		INFLUX_RENDER_API const world& get_world() const;

		INFLUX_RENDER_API bool is_empty() const;

		INFLUX_RENDER_API result<matrix> get_transform( const transform_id& id ) const;

		INFLUX_RENDER_API const vector<mesh_instance>& get_mesh_instances() const;

		INFLUX_RENDER_API const vector<line>& get_lines() const;

		INFLUX_RENDER_API const vector<light_instance>& get_lights() const;

		INFLUX_RENDER_API uint32 get_num_lights_total() const;

		INFLUX_RENDER_API uint32 get_num_lights(e_light_type type) const;

		INFLUX_RENDER_API const world_constants& get_world_constants() const;

		INFLUX_RENDER_API const view_matrices& get_view_matrices() const;

		INFLUX_RENDER_API const matrix& get_camera_transform() const;

		INFLUX_RENDER_API bool is_debug_render_enabled() const;

		INFLUX_RENDER_API bool has_debug_primitives() const;

		worldview() = default;
	};

	struct sprite2D final
	{
		math::transform2D m_transform;
		math::rectf m_rectangle;
		string m_texture;

		bool m_scale_to_view;
	};

	struct scene_imgui final
	{
		inline bool is_empty() const
		{
			return m_imgui_stacks.size() == 0u;
		}

		vector<function<void(ImGuiContext&)>> m_imgui_stacks{};
	};
}