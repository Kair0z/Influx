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
			: m_name{ name }, m_transform{ transform }, m_material_name{ mat_name }, m_per_instance_colour{ colour } {}

		string m_name = "";
		string m_material_name = "";
		math::vectorf4 m_per_instance_colour = {};
		math::matrix4x4f m_transform = math::matrix4x4f::identity();
		bool m_invert_normals = false;
	};

	struct scene final
	{
		scene() = default;
		scene(const vector<mesh_instance>& meshes, const camera& camera);

		uint32	get_num_meshes() const;
		bool	has_meshes() const;

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
			math::float3 m_points[2]{};
			math::colour_rgba m_colour;
		};

		void add_box(const math::boxf& box, const math::colour_rgba& colour)
		{
			
		}

		void add_line(const line& line, const math::colour_rgba& colour)
		{
			m_lines.push_back(line);
		}

		void add_point(const math::float3& point, const math::colour_rgba& colour)
		{
			add_line({ point, point });
		}

		vector<line> m_lines;
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