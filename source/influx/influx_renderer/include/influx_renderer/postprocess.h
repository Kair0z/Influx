#pragma once 

#include "core/basetypes.h"
#include "core/enum.h"

namespace influx::renderer
{
	enum class postprocess_flags : uint8
	{
		none		= 0,
		bloom		= 1 << 0,
		blur		= 1 << 1,
		vignette	= 1 << 2
	};

	constexpr uint32 k_max_num_postprocess_layers = 3u;

	struct scene_postprocess final
	{
		postprocess_flags m_flags = postprocess_flags::none;

		uint32 get_num_active_layers() const
		{
			uint32 count = 0u;
			for (uint32 i = 0u; i < k_max_num_postprocess_layers; ++i)
			{
				count += is_enabled(static_cast<postprocess_flags>(1 << i)) ? 1u : 0u;
			}
			return count;
		}

		bool is_enabled(postprocess_flags flag) const
		{
			return has_flag(m_flags, flag);
		}
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::renderer::postprocess_flags)