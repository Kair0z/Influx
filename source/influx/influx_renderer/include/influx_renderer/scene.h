#pragma once

// influx::core
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/transform.h"
#include "core/string.h"
#include "core/container/vector.h"
#include "core/math/colour.h"
#include "core/geometry/rect.h"
#include "core/math/bounds.h"
#include "core/math/transform.h"
#include "core/material/material.h"
#include "core/scene/light.h"

// influx::renderer
#include "types.h"

// imgui
struct ImGuiContext;

namespace influx::renderer
{
	struct scene;

	static constexpr uint32 k_invalid_id = (uint32)-1;
	using material_id	= uint32;
	using camera_id		= uint32;
	using mesh_inst_id	= uint32;
	using mesh_id		= uint32;
	using light_id		= uint32;
	using transform_id	= uint32;

	struct light final
	{
		light_id m_id = k_invalid_id;
		influx::light m_light;
		math::float3 m_world_position;
		math::float3 m_world_forward;
	};

	struct camera final
	{
		camera_id m_id = k_invalid_id;
		float m_fov = 90.0f;
		float m_near_plane	= 0.0001f;
		float m_far_plane	= 1000.0f;
		transform_id m_transform_id = k_invalid_id;
	};

	struct view_matrices final
	{
		view_matrices() = default;
		explicit view_matrices(const math::matrix4x4f& transform, const camera& camera);

		math::matrix4x4f m_transform;			// transform of the camera
		math::matrix4x4f m_projection;
		math::matrix4x4f m_view;				// inv transform
		math::matrix4x4f m_viewprojection;
		math::matrix4x4f m_inv_viewprojection;
		math::matrix4x4f m_inv_projection;
	};

	struct mesh_instance final
	{
		mesh_id	m_mesh_id			= k_invalid_id;
		material_id	m_mat_id		= k_invalid_id;
		transform_id m_transform_id	= k_invalid_id;
		math::vectorf4 m_per_instance_colour = {};

		const math::matrix4x4f& get_transform(const scene&) const;
	};

	struct scene final
	{
	public:
		scene() = default;
		
		INFLUX_RENDER_API
		bool is_empty() const;

		// adding transforms
		INFLUX_RENDER_API
		transform_id add_transform(const math::matrix4x4f& matrix);
		
		INFLUX_RENDER_API
		math::matrix4x4f& get_transform(const transform_id& id);
		
		INFLUX_RENDER_API
		const math::matrix4x4f& get_transform(const transform_id& id) const;

		// adding meshes
		INFLUX_RENDER_API 
		mesh_instance& add_mesh(const mesh_id& mesh_id, const math::matrix4x4f& transform = math::matrix4x4f::identity());
		INFLUX_RENDER_API 
		mesh_instance& get_mesh(const mesh_inst_id& id);
		INFLUX_RENDER_API 
		mesh_instance& get_last_mesh();
		INFLUX_RENDER_API 
		uint32 get_num_meshes() const;
		INFLUX_RENDER_API 
		bool has_meshes() const;
		INFLUX_RENDER_API 
		const vector<mesh_instance>& get_meshes() const;
		
		// adding lights
		light& add_light(const influx::light& light, const math::matrix4x4f& transform = math::matrix4x4f::identity());
		uint32 get_num_lights() const;
		uint32 get_num_lights(influx::e_light_type type) const;
		bool has_lights() const;
		const vector<light>& get_lights() const;

		const camera& get_camera() const;
		const math::matrix4x4f& get_camera_transform() const;
		void set_camera(const camera& camera, const math::matrix4x4f& transform);
		const view_matrices& get_view_matrices() const;

		float m_delta_seconds;
		float m_seconds;

	private:
		vector<math::matrix4x4f>	m_transforms{};
		vector<mesh_instance>		m_meshes = {};
		vector<light>				m_lights = {};
		camera						m_camera = {};
		view_matrices				m_viewmatrices{};
	};

	struct sprite2D final
	{
		math::transform2D m_transform;
		math::rectf m_rectangle;
		string m_texture;

		bool m_scale_to_view;
	};

	struct scene2D final
	{
		inline bool is_empty() const
		{
			return m_sprites.size() == 0u;
		}

		vector<sprite2D> m_sprites = {};
	};

	struct scene_imgui final
	{
		inline bool is_empty() const
		{
			return m_imgui_stacks.size() == 0u;
		}

		vector<function<void(ImGuiContext&)>> m_imgui_stacks{};
	};

	struct scene_debug final
	{
	public:
		inline bool is_empty() const
		{
			return m_lines.size() == 0u;
		}

		struct line final
		{
			line(const math::float3& start, const math::float3& end, const math::colour_rgba& colour)
				: m_points{start, end}
				, m_colour{ colour } {}

			math::float3 m_points[2]{};
			math::colour_rgba m_colour;
		};

		void add_box(const math::boxf& box, const math::colour_rgba& colour)
		{
			const math::vectorf3& max = box.get_maximum();
			const math::vectorf3& min = box.get_minimum();
			const math::vectorf3& mid = box.get_middle();
			const math::vectorf3 dimensions = max - min;

			// bottom square
			const math::float3 bottoms[4] =
			{
				min,
				min + dimensions * math::float3{+1.0f,0.0f,0.0f},
				min + dimensions * math::float3{0.0f,0.0f,+1.0f},
				min + dimensions * math::float3{+1.0f,0.0f,+1.0f}
			};
			add_line(bottoms[0], bottoms[1], colour);
			add_line(bottoms[0], bottoms[2], colour);
			add_line(bottoms[2], bottoms[3], colour);
			add_line(bottoms[1], bottoms[3], colour);

			// top square
			const math::float3 tops[4] =
			{
				max + dimensions * math::float3{-1.0f,0.0f,-1.0f},
				max + dimensions * math::float3{0.0f,0.0f,-1.0f},
				max + dimensions * math::float3{-1.0f,0.0f,0.0f},
				max
			};
			add_line(tops[0], tops[1], colour);
			add_line(tops[0], tops[2], colour);
			add_line(tops[2], tops[3], colour);
			add_line(tops[1], tops[3], colour);

			// vertical lines
			for (uint8 i = 0u; i < 4u; ++i)
			{
				add_line(bottoms[i], tops[i], colour);
			}
		}

		void add_line(const line& line)
		{
			m_lines.push_back(line);
		}

		void add_line(const math::float3& start, const math::float3& end, const math::colour_rgba& colour)
		{
			add_line({ start, end, colour});
		}
		
		void add_point(const math::float3& point, const math::colour_rgba& colour)
		{
			add_line({ point, point, colour });
		}

		void add_gizmo_transform(const math::transform3D& transform)
		{
			const math::float3& position = transform.get_position();
			add_line(position, position + transform.get_right(), { 1,0,0,1 });
			add_line(position, position + transform.get_up(), { 0,1,0,1 });
			add_line(position, position + transform.get_forward(), { 0,0,1,1 });
		}

		void clear()
		{
			m_lines.clear();
		}

		void set_camera(const camera& camera, const math::matrix4x4f& transform);

		const view_matrices& get_view_matrices() const;

		const vector<line>& get_lines() const;

	private:
		vector<line> m_lines{};
		camera m_camera = {};
		view_matrices m_viewmatrices{};
	};
}