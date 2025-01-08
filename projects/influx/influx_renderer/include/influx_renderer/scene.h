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

// influx::renderer
#include "types.h"

namespace influx::renderer
{
	struct camera final
	{
		float m_fov = 90.0f;
		float m_near_plane = 0.0001f;
		float m_far_plane = 100.0f;

		math::transform3D m_transform;
	};

	struct mesh_instance final
	{
		mesh_instance() = default;
		mesh_instance(const string& name, const math::matrix4x4f& transform, const string& mat_name, const math::vectorf4& colour)
			: m_name{ name }, m_transform{ transform }, m_per_instance_colour{ colour } {}

		string m_name = "";
		math::vectorf4 m_per_instance_colour = {};
		math::matrix4x4f m_transform = math::matrix4x4f::identity();
		const material* m_material = nullptr;
	};

	struct scene final
	{
		scene() = default;
		scene(const vector<mesh_instance>& meshes, const camera& camera);

		uint32	get_num_meshes() const;
		bool	has_meshes() const;

		uint32 get_num_materials() const;
		bool has_materials() const;

		vector<mesh_instance> m_meshes = {};
		camera m_camera = {};

		float m_delta_seconds;
		float m_seconds;
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
		vector<sprite2D> m_sprites = {};
	};

	struct scene_debug final
	{
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

		vector<line> m_lines{};
		camera m_camera = {};
	};
}

namespace influx::renderer
{
	inline scene::scene(const vector<mesh_instance>& meshes, const camera& camera)
		: m_meshes{ meshes }
		, m_camera{ camera }
	{
	}

	inline uint32 scene::get_num_meshes() const
	{
		return static_cast<uint32>(m_meshes.size());
	}

	inline bool scene::has_meshes() const
	{
		return get_num_meshes() > 0u;
	}
}