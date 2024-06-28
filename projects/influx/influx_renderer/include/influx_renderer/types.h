#pragma once
#include "core/basetypes.h"

namespace influx::renderer
{
	// num backbuffers held by a swapchain
	enum class e_buffering : uint8
	{
		dubble = 2,
		tripple = 3,
		max
	};

	// graphics api
	enum class e_render_api : uint8
	{
		dx12,
		unsupported,
		vulkan,
		max
	};
}