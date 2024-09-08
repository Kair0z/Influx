#pragma once

// influx::core
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/transform.h"
#include "core/string.h"
#include "core/container/vector.h"
#include "core/math/colour.h"

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
	};

	struct scene final
	{
		scene() = default;
		scene(const vector<mesh_instance>& meshes, const camera& camera)
			: m_meshes{ meshes },
			m_camera{ camera } {}

		vector<mesh_instance> m_meshes = {};
		camera m_camera = {};

		float m_delta_seconds;
		float m_seconds;
	};
}