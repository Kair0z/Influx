#pragma once
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::renderer
{
	struct texture_data final
	{
		vector<math::vectorf4> m_pixels{};
		uint32 m_width = 0u;

		uint32 get_width() const
		{
			return m_width;
		}

		uint32 get_height() const
		{
			return static_cast<uint32>(m_pixels.size()) / get_width();
		}

		bool is_valid() const;
	};
}