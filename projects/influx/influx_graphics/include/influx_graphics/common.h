#pragma once
#include "core/string.h"
#include "core/basetypes.h"

namespace influx::graphics
{
	constexpr static uint8 k_max_render_targets = 8u;

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
		d32 = 1,
		count
	};

	enum class e_comparison_func : uint8
	{
		lequal,		// less than equal
		count
	};

	enum class e_primitive_topology_type : uint8
	{
		triangle,
		count
	};

	enum class e_primitive_topology : uint8
	{
		trilist,
		count
	};

	struct viewport
	{

	};

	struct scissor_rect
	{

	};
}