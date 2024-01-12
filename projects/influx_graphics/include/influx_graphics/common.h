#pragma once

#include "core/string.h"
#include "core/basetypes.h"

namespace influx::graphics
{
	// graphics api
	enum class e_api_type : uint8
	{
		dx12,
		unsupported,
		vulkan,
		max
	};

	struct physical_device_info final
	{
		string m_name{};
	};

	enum class e_format : uint8
	{
		rgba8 = 0,
		count
	};
}