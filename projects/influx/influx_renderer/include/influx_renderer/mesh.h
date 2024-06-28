#pragma once
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::renderer
{
	using index = uint32;

	struct vertex_data final
	{
		math::vectorf3 m_position{};
		math::vectorf4 m_colour{};
		math::vectorf3 m_normal{};
		math::vectorf2 m_texcoords{};
	};

	struct mesh_data final
	{
		vector<vertex_data> m_vertices{};
		vector<index> m_indices{};

		bool is_valid() const;
	};
}