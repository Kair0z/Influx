#pragma once

#include "Core/basetypes.h"
#include "Core/Math/Transform.h"

namespace influx::application
{
	struct entity final
	{
		entity() = default;
		entity(uint64 id) : m_id{ id } {}

		uint64 m_id = 0u;
		math::transform3D m_transform = math::transform3D::identity();
	};

	class scene
	{
	public:

	};
}


