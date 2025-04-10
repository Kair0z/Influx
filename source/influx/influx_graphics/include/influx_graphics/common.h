#pragma once

#if _DLL
#define INFLUX_GFX_API __declspec(dllexport)
#else
#define INFLUX_GFX_API __declspec(dllimport)
#endif

// influx::core
#include "core/string.h"
#include "core/basetypes.h"
#include "core/enum.h"
#include "core/result.h"

namespace influx::graphics
{
	enum class e_log { info, warning, error, count };
	typedef void (*log_function)(e_log, const char*);

	template <typename _t>
	using result = influx::result<_t, const char*>;

	enum class e_feature_flags : uint8
	{
		none				= 0 << 0,
		raytracing			= 1 << 0,
		all = raytracing
	};

	struct feature_info final
	{
		e_feature_flags m_supported_flags;
	};

	enum class e_command : uint32
	{
		none,
		draw_any,

		set_rtv,
		set_srv,
		set_descriptor_heap,
		set_root_constants,
		set_indexbuffer,
		set_vertexbuffer,

		clear_dsv,
		clear_rtv,
		clear_state,
		clear_uav,

		/* raytracing acceleration structures */
		build_as,

		/* mesh shaders */
		dispatch_mesh,

		/* copy */
		copy_any,
		copy_buffer,
		copy_resource,
		copy_texture,
		copy_tiles,
		atomic_copy_buffer,

		discard,
		dispatch,
		
		barrier_any,
		barrier_transition,

		resolve_any,
		begin_renderpass,
		count
	};

	enum class e_resource_state : uint32
	{
		none			= 0 << 0,
		common			= 1 << 0,
		present			= 1 << 1,
		render_target	= 1 << 2,
		depth_target	= 1 << 3,
		depth_readonly	= 1 << 4,
		vs_srv			= 1 << 5,
		ps_srv			= 1 << 6,
		cs_srv			= 1 << 7,
		vs_uav			= 1 << 8,
		ps_uav			= 1 << 9,
		cs_uav			= 1 << 10,
		clear_uav		= 1 << 11,
		copy_src		= 1 << 12,
		copy_dst		= 1 << 13,
		shading_rate	= 1 << 14,
		indexbuffer		= 1 << 15,
		indirect_args	= 1 << 16,
		as_read			= 1 << 17,
		as_write		= 1 << 18,
		discard			= 1 << 19,
		resolve_dst		= 1 << 20,
		resolve_src		= 1 << 21,

		all_vs = vs_srv | vs_uav,
		all_ps = ps_srv | ps_uav,
		all_cs = cs_srv | cs_uav,
		all_srv = vs_srv | ps_srv | cs_srv,
		all_uav = vs_uav | ps_uav | cs_uav,
		all_depth = depth_target | depth_readonly,
		all_copy = copy_src | copy_dst,
		all_as = as_read | as_write,
		gen_read = copy_src | all_srv,
		gen_write = copy_dst | all_uav,
		all_shading = all_srv | all_uav | shading_rate | as_read
	};

	enum class e_pipeline_type : uint8
	{
		graphics,
		compute,
		raytracing,
		mesh,
		count
	};
	constexpr static uint8 k_num_pipeline_types = static_cast<uint8>(e_pipeline_type::count);

	constexpr static uint8 k_max_render_targets = 8u;

	// graphics api
	enum class e_api_type : uint8
	{
		dx12,
		vulkan,
		unsupported,
		max
	};

	struct physical_device_info final
	{
		string m_name{};
	};

	struct memory_info final
	{
		size_t m_gpu_budget{};
		size_t m_gpu_usage{}; // in bytes
	};

	enum class e_commandlist_type
	{
		graphics,
		compute,
		count
	};

	enum class e_format : uint8
	{
		rgba8,
		r32,
		rg32,
		rgb32,
		rgba32,
		d32,
		u16,
		u32,
		rgba_u32,
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
		case e_format::u16:		return 1u * 2u;
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

	// D3D12_FILL_MODE
	// ...
	enum class e_fill_mode : uint8
	{
		solid,
		wireframe,
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

	// D3D12_BLEND
	// ...
	enum class e_blend : uint8
	{
		zero = 1,
		one = 2,
		src_color = 3,
		inv_src_color = 4,
		src_alpha = 5,
		inv_src_alpha = 6,
		dest_alpha = 7,
		inv_dest_alpha = 8,
		dest_color = 9,
		inv_dest_color = 10,
		src_alpha_sat = 11,
		blend_factor = 14,
		inv_blend_factor = 15,
		src1_color = 16,
		inv_src1_color = 17,
		src1_alpha = 18,
		inv_src1_alpha = 19,
		alpha_factor = 20,
		inv_alpha_factor = 21,
		count
	};

	// D3D12_BLEND_OP
	// ...
	enum class e_blendop : uint8
	{
		add = 1,
		subtract = 2,
		rev_subtract = 3,
		min = 4,
		max = 5,
		count
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
		none		= 0,
		vertex		= 1 << 0,
		hull		= 1 << 1,
		domain		= 1 << 2,
		geometry	= 1 << 3,
		pixel		= 1 << 4,
		compute		= 1 << 5,

		all			= vertex | hull | domain | geometry | pixel | compute
	};

	enum class e_bind_flags : uint32
	{
		none	= 0,
		srv		= 1 << 0,
		rtv		= 1 << 1,
		dsv		= 1 << 2,
		uav		= 1 << 3
	};

	// D3D12_PRIMITIVE_TOPOLOGY_TYPE
	enum class e_primitive_topology_type : uint8
	{
		triangle	= 0,
		point		= 1,
		line		= 2,
		patch		= 3,
		count
	};

	// D3D_PRIMITIVE_TOPOLOGY
	enum class e_primitive_topology : uint8
	{
		trilist,
		linelist,
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

ENABLE_ENUM_BIT_OPERATORS(influx::graphics::e_feature_flags);
ENABLE_ENUM_BIT_OPERATORS(influx::graphics::e_command);
ENABLE_ENUM_BIT_OPERATORS(influx::graphics::e_resource_state);
ENABLE_ENUM_BIT_OPERATORS(influx::graphics::e_bind_flags);
ENABLE_ENUM_BIT_OPERATORS(influx::graphics::e_shader_visibility);