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

	constexpr static uint8 get_num_buffers(e_buffering buffering)
	{
		switch (buffering)
		{
		case e_buffering::dubble: return 2u;
		case e_buffering::tripple: return 3u;
		default:
		case e_buffering::max: return (uint8)-1;
		}
	}

	// graphics api
	enum class e_render_api : uint8
	{
		dx12,
		unsupported,
		vulkan,
		max
	};
}