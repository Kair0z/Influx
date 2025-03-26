#include "renderer_pch.h"
#include "influx_renderer/scene.h"

namespace influx::renderer
{
	bool scene::is_empty() const
	{
		return !has_meshes();
	}

	uint32 scene::get_num_meshes() const
	{
		return static_cast<uint32>(m_meshes.size());
	}

	bool scene::has_meshes() const
	{
		return get_num_meshes() > 0u;
	}

	const vector<mesh_instance>& scene::get_meshes() const
	{
		return m_meshes;
	}

	transform_id scene::add_transform(const math::matrix4x4f& matrix)
	{
		m_transforms.push_back(matrix);
		return static_cast<transform_id>(m_transforms.size() - 1u);
	}

	math::matrix4x4f& scene::get_transform(const transform_id& id)
	{
		return m_transforms[id];
	}

	const math::matrix4x4f& scene::get_transform(const transform_id& id) const
	{
		return m_transforms[id];
	}

	mesh_instance& scene::add_mesh(const mesh_id& mesh_id, const math::matrix4x4f& transform)
	{
		mesh_instance new_instance{};
		new_instance.m_transform_id = add_transform(transform);
		new_instance.m_mat_id = 0u;
		new_instance.m_per_instance_colour;
		new_instance.m_mesh_id = mesh_id;
		m_meshes.push_back(new_instance);
		return m_meshes.back();
	}

	mesh_instance& scene::get_mesh(const mesh_inst_id& id)
	{
		return m_meshes[id];
	}
	mesh_instance& scene::get_last_mesh()
	{
		return m_meshes.back();
	}
	light& scene::add_light(const influx::light& _light, const math::matrix4x4f& transform)
	{
		light result{};
		result.m_light = _light;
		result.m_transform_id = add_transform(transform);
		m_lights.push_back(result);
		return m_lights.back();
	}
	uint32 scene::get_num_lights() const
	{
		return static_cast<uint32>(m_lights.size());
	}
	uint32 scene::get_num_lights(influx::e_light_type type) const
	{
		uint32 count = 0u;
		for (const light& light : m_lights)
		{
			if (light.m_light.get_type() == type) count++;
		}
		return count;
	}
	bool scene::has_lights() const
	{
		return false;
	}
	const vector<light>& scene::get_lights() const
	{
		return m_lights;
	}
	const camera& scene::get_camera() const
	{
		return m_camera;
	}
	const math::matrix4x4f& scene::get_camera_transform() const
	{
		return m_transforms[m_camera.m_transform_id];
	}

	void scene::set_camera(const camera& camera, const math::matrix4x4f& transform)
	{
		set_camera(camera);

		m_camera.m_transform_id = add_transform(transform);
		m_viewmatrices = view_matrices(get_transform(m_camera.m_transform_id), camera);
	}

	void scene::set_camera(const camera& camera)
	{
		// preserve the existing transform id!
		transform_id trans_id = m_camera.m_transform_id;
		m_camera = camera;
		m_camera.m_transform_id = trans_id;
	}

	void scene::set_camera_transform(const math::matrix4x4f& transform)
	{
		math::matrix4x4f& matrix = get_transform(m_camera.m_transform_id);
		matrix = transform;
	}

	const view_matrices& scene::get_view_matrices() const
	{
		return m_viewmatrices;
	}

	void scene::add_line_box(const math::boxf& box, const math::colour_rgba& colour)
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

	void scene::add_line(const line& line)
	{
		m_lines.push_back(line);
	}

	void scene::add_line(const math::float3& start, const math::float3& end, const math::colour_rgba& colour)
	{
		add_line({ start, end, colour });
	}

	void scene::add_point(const math::float3& point, const math::colour_rgba& colour)
	{
		add_line({ point, point, colour });
	}

	void scene::add_gizmo_transform(const math::transform3D& transform)
	{
		const math::float3& position = transform.get_position();
		add_line(position, position + transform.get_right(), { 1,0,0,1 });
		add_line(position, position + transform.get_up(), { 0,1,0,1 });
		add_line(position, position + transform.get_forward(), { 0,0,1,1 });
	}

	const vector<line>& scene::get_lines() const
	{
		return m_lines;
	}

	bool scene::has_debug_primitives() const
	{
		return !m_lines.empty();
	}

	bool scene::is_debug_render_enabled() const
	{
		return has_flag(m_renderflags, e_scene_render_flags::enable_debug);
	}

	void scene::set_debug_render_enabled(bool enabled)
	{
		set_flag(m_renderflags, e_scene_render_flags::enable_debug, enabled);
	}

	view_matrices::view_matrices(const math::matrix4x4f& transform, const camera& camera)
	{
		float aspect_ratio = 0.0f;
		m_transform = transform;
		m_projection = camera.m_camera.get_projection();
		m_view = transform.inverted();
		m_view.set_column(2u, -m_view.get_column(2u));
		m_viewprojection = m_view * m_projection;
		m_inv_viewprojection = m_viewprojection.inverted();
		m_inv_projection = m_projection.inverted();
	}
}