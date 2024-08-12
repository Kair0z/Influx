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
		rgba8,
		r32,
		rg32,
		rgb32,
		rgba32,
		d32,
		u32,
		count
	};

	constexpr size_t deduce_bytesize(e_format format)
	{
		switch (format)
		{
		case e_format::d32:		return 1u * 4u;
		case e_format::r32:		return 1u * 4u;
		case e_format::rg32:	return 2u * 4u;
		case e_format::rgb32:	return 3u * 4u;
		case e_format::rgba32:	return 4u * 4u;
		case e_format::rgba8:	return 4u * 1u;
		case e_format::u32:		return 1u * 4u;
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
		always,
		count
	};

	// D3D12_STATIC_BORDER_COLOR
	enum class e_border_color : uint8
	{
		black,
		white,
		black_transparent,
		count
	};

	// D3D12_FILTER
	// ...
	enum class e_filter : uint32
	{
		min_mag_mip_point = 0,
		min_mag_point_mip_linear = 0x1,
		min_point_mag_linear_mip_point = 0x4,
		min_point_mag_mip_linear = 0x5,
		min_linear_mag_mip_point = 0x10,
		min_linear_mag_point_mip_linear = 0x11,
		min_mag_linear_mip_point = 0x14,
		min_mag_mip_linear = 0x15,
		anisotropic = 0x55,
		comparison_min_mag_mip_point = 0x80,
		comparison_min_mag_point_mip_linear = 0x81,
		comparison_min_point_mag_linear_mip_point = 0x84,
		comparison_min_point_mag_mip_linear = 0x85,
		comparison_min_linear_mag_mip_point = 0x90,
		comparison_min_linear_mag_point_mip_linear = 0x91,
		comparison_min_mag_linear_mip_point = 0x94,
		comparison_min_mag_mip_linear = 0x95,
		comparison_anisotropic = 0xd5,
		minimum_min_mag_mip_point = 0x100,
		minimum_min_mag_point_mip_linear = 0x101,
		minimum_min_point_mag_linear_mip_point = 0x104,
		minimum_min_point_mag_mip_linear = 0x105,
		minimum_min_linear_mag_mip_point = 0x110,
		minimum_min_linear_mag_point_mip_linear = 0x111,
		minimum_min_mag_linear_mip_point = 0x114,
		minimum_min_mag_mip_linear = 0x115,
		minimum_anisotropic = 0x155,
		maximum_min_mag_mip_point = 0x180,
		maximum_min_mag_point_mip_linear = 0x181,
		maximum_min_point_mag_linear_mip_point = 0x184,
		maximum_min_point_mag_mip_linear = 0x185,
		maximum_min_linear_mag_mip_point = 0x190,
		maximum_min_linear_mag_point_mip_linear = 0x191,
		maximum_min_mag_linear_mip_point = 0x194,
		maximum_min_mag_mip_linear = 0x195,
		maximum_anisotropic = 0x1d5
	};

	// D3D12_TEXTURE_ADDRESS_MODE
	// ...
	enum class e_texture_wrap_mode : uint8
	{
		wrap,
		mirror,
		clamp,
		border,
		mirror_once,
		count
	};

	enum class e_shader_visibility : uint32
	{
		all = 0,
		vertex = 1,
		hull = 2,
		domain = 3,
		geometry = 4,
		pixel = 5,
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