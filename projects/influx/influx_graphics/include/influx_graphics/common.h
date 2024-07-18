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
		u32 = 2,
		count
	};

	constexpr size_t deduce_bytesize(e_format format)
	{
		switch (format)
		{
		case e_format::d32: return 4u;
		case e_format::rgba8: return 4u * 8u;
		case e_format::u32: return 4u;
		default:
		case e_format::count: return (size_t)-1;
		}
	}

	enum class e_cull_mode : uint8
	{
		front,
		back,
		nocull,
		count
	};

	enum class e_comparison_func : uint8
	{
		less,		// <
		lequal,		// <=
		gequal,		// >=
		greater,	// >
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

	struct viewport final
	{
		float m_left;
		float m_top;
		float m_width;
		float m_height;
		float m_depth_min;
		float m_depth_max;
	};

	struct rect final
	{
		uint32 m_left;
		uint32 m_top;
		uint32 m_right;
		uint32 m_bottom;
	};
}