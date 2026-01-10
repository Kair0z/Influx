#pragma once

// influx::renderer
#include "influx_renderer/common.h"

namespace influx::renderer
{
	// misc
	constexpr static bool	k_useWarp = true;
	constexpr static uint8	k_num_srvs_max = 64u;
	
	// swapchain
	constexpr static e_buffering k_swapchain_buffering_default = e_buffering::tripple;
	constexpr static uint8 k_num_swapchain_buffers_max = static_cast<uint8>(e_buffering::max) - 1u;
	constexpr static uint8 k_num_swapchain_buffers_def = static_cast<uint8>(k_swapchain_buffering_default);

	// threading
	constexpr static uint8 k_num_inflight_max = k_num_swapchain_buffers_def;

	// stats
	constexpr static uint32 k_num_statframes_max = 2048u;

	static constexpr shader::e_shader_target k_shader_target = shader::e_shader_target::_6_6;

	enum class e_internal_texture : uint8
	{
		none,
		num
	};
}