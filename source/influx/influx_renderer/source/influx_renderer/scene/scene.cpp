#include "renderer_pch.h"
#include "influx_renderer/scene.h"

namespace influx::renderer
{
#pragma region world
	mesh_instance_id world::add_mesh_instance(
		const mesh_id& mesh,
		const matrix& transform,
		const material_id& material,
		const colour& colour)
	{
		mesh_instance new_instance{};
		new_instance.m_transform_id = add_transform(transform);
		new_instance.m_mesh_id = mesh;
		new_instance.m_per_instance_colour = colour;
		new_instance.m_mat_id = material;
		m_meshes.push_back(new_instance);
		return (uint32)m_meshes.size() - 1u;
	}

	light_instance_id world::add_light(const light& light, const matrix& transform)
	{
		light_instance new_instance{};
		new_instance.m_transform = add_transform(transform);
		new_instance.m_light = light;
		m_lights.push_back(new_instance);
		return (uint32)m_lights.size() - 1u;
	}

	transform_id world::add_transform(const matrix& mat)
	{
		m_transforms.push_back(mat);
		return (uint32) m_transforms.size() - 1;
	}

	result<matrix> world::get_transform(const transform_id& id) const
	{
		using result_type = result<matrix>;
		if (!in_range(id))
			return result_type::make_error("id is out of range!");

		return m_transforms[id];
	}

	result<matrix> worldview::get_transform(const transform_id& id) const
	{
		using result_type = result<matrix>;
		if (m_world == nullptr)
			return result_type::make_error("world is nullptr!");

		return m_world->get_transform(id);
	}

	const world& worldview::get_world() const
	{
		return *m_world;
	}
	bool worldview::is_empty() const
	{
		return m_world == nullptr || m_world->m_meshes.size() == 0u;
	}

	const vector<mesh_instance>& world::get_mesh_instances() const
	{
		return m_meshes;
	}

	const vector<mesh_instance>& worldview::get_mesh_instances() const
	{
		if (m_world == nullptr)
		{
			static const vector<mesh_instance> def_meshes{};
			return def_meshes;
		}
		return m_world->m_meshes;
	}

	const vector<line>& world::get_lines() const
	{
		return m_lines;
	}

	const vector<light_instance>& world::get_lights() const
	{
		return m_lights;
	}

	const vector<light_instance>& worldview::get_lights() const
	{
		if (m_world == nullptr)
		{
			static const vector<light_instance> instances{};
			return instances;
		}
		return m_world->get_lights();
	}

	uint32 world::get_num_lights_total() const
	{
		return static_cast<uint32>(m_lights.size());
	}

	uint32 worldview::get_num_lights_total() const
	{
		if (m_world == nullptr)
			return 0u;

		return m_world->get_num_lights_total();
	}

	uint32 world::get_num_lights(e_light_type type) const
	{
		uint32 sum = 0u;
		for (const auto& light_instance : get_lights())
		{
			if (light_instance.m_light.get_type() == type)
				sum++;
		}
		return sum;
	}

	uint32 worldview::get_num_lights(e_light_type type) const
	{
		if (m_world == nullptr)
			return {};

		return m_world->get_num_lights(type);
	}

	const world_constants& world::get_world_constants() const
	{
		return m_constants;
	}

	const world_constants& worldview::get_world_constants() const
	{
		if (m_world == nullptr)
		{
			static const world_constants def{};
			return def;
		}
		return m_world->m_constants;
	}

	const view_matrices& worldview::get_view_matrices() const
	{
		return m_matrices;
	}

	const matrix& worldview::get_camera_transform() const
	{
		return get_view_matrices().m_transform;
	}

	bool worldview::is_debug_render_enabled() const
	{
		return has_flag(m_renderflags, e_scene_render_flags::enable_debug);
	}

	bool worldview::has_debug_primitives() const
	{
		return get_lines().size() > 0u;
	}

	const vector<line>& worldview::get_lines() const
	{
		if (m_world == nullptr)
		{
			static const vector<line> lines{};
			return lines;
		}
		return m_world->get_lines();
	}
#pragma endregion

#if 0
	bool scene::is_empty() const
	{
		return !has_camera();
	}

	uint32 scene::get_num_meshes() const
	{
		return static_cast<uint32>(m_meshes.size());
	}

	bool scene::has_meshes() const
	{
		return get_num_meshes() > 0u;
	}

	bool scene::has_camera() const
	{
#if 0
		bool valid_transform = m_camera.m_transform_id != k_invalid_id;
		bool valid_settings = m_camera.m_camera.get_fov() > 0.0f;
		return valid_transform && valid_settings;
#endif
		return false;
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

	mesh_instance& scene::add_mesh(const mesh_id& mesh_id, const matrix& transform)
	{
		mesh_instance new_instance{};
		new_instance.m_transform_id = add_transform(transform);
		new_instance.m_mat_id = 0u;
		new_instance.m_per_instance_colour;
		new_instance.m_mesh_id = mesh_id;
		m_meshes.push_back(new_instance);
		return m_meshes.back();
	}

	mesh_instance& scene::add_mesh(e_mesh mesh, const matrix& transform)
	{
		// renderer::get_internal_mesh_name(mesh)
		return add_mesh(0, transform);
	}

	mesh_instance& scene::get_mesh(const mesh_instance_id& id)
	{
		return m_meshes[id];
	}
	mesh_instance& scene::get_last_mesh()
	{
		return m_meshes.back();
	}
	light& scene::add_light(const influx::light& _light, const matrix& transform)
	{
		light result = _light;
		// result.m_light = _light;
		// result.m_transform_id = add_transform(transform);
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
			if (light.get_type() == type) count++;
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
	result<math::matrix4x4f> scene::get_camera_transform() const
	{
#if 0
		using result_type = result<math::matrix4x4f>;
		if (m_camera.m_transform_id == k_invalid_id)
			return result_type::make_error("error: no camera transform set, probably no valid camera!");

		return m_transforms[m_camera.m_transform_id];
#endif
		return {};
	}

	void scene::set_camera(const influx::camera& camera, const math::matrix4x4f& transform)
	{
		set_camera_settings(camera);
		set_camera_transform(transform);
	}

	void scene::set_camera_settings(const influx::camera& camera)
	{
#if 0
		m_camera.m_camera = camera;
#endif
	}

	void scene::set_camera_transform(const math::matrix4x4f& transform)
	{
#if 0
		if (m_transforms.empty())
		{
			m_transforms.push_back(transform);
			m_camera.m_transform_id = 0u;

			m_viewmatrices = view_matrices(transform, m_camera);
			return;
		}
		
		if (m_camera.m_transform_id == k_invalid_id)
		{
			m_camera.m_transform_id = add_transform(transform);
		}
		else
		{
			vector_helpers::grow(m_transforms, m_camera.m_transform_id);
			m_transforms[m_camera.m_transform_id] = transform;
		}

		// update viewmatrices
		m_viewmatrices = view_matrices(transform, m_camera);
#endif
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
		m_transform			= transform;
#if 0
		m_projection		= camera.m_camera.get_projection();
		m_view				= camera.m_camera.get_view(transform);
#endif
		m_viewprojection	= m_view * m_projection;
		m_inv_viewprojection = m_viewprojection.inverted();
		m_inv_projection	= m_projection.inverted();
	}
#endif
}