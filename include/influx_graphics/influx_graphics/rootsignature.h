#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

namespace influx::graphics
{
	enum class e_rootparam_type : uint32
	{
		descriptor_table,
		constants,
		cbv,
		srv,
		uav,
		count
	};

	struct root_param_common final
	{
		e_shader_visibility m_visibility = e_shader_visibility::all;
		uint32 m_shader_register = 0u;
		uint32 m_register_space = 0u;
	};

	struct root_param_constants final
	{
		root_param_constants() = default;
		root_param_constants(
			uint32 num_dwords,
			uint32 shader_register,
			uint32 register_space = 0,
			e_shader_visibility visibility = e_shader_visibility::all)
		{
			m_num_dwords = num_dwords;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
			m_common.m_visibility = visibility;
		}

		root_param_common m_common;
		uint32 m_num_dwords;
	};

	struct root_param_resource final
	{
		enum class e_type : uint8
		{
			srv,
			cbv,
			uav,
			count
		};

		root_param_resource() = default;
		root_param_resource(
			e_type type,
			uint32 shader_register,
			uint32 register_space = 0,
			e_shader_visibility visibility = e_shader_visibility::all)
		{
			m_type = type;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
			m_common.m_visibility = visibility;
		}

		e_type m_type;
		root_param_common m_common;
	};

	struct root_param_resource_range final
	{
		enum class e_type : uint8
		{
			srv,
			cbv,
			uav,
			sampler,
			count
		};

		root_param_resource_range() = default;
		root_param_resource_range(
			uint32 num_resources,
			e_type type,
			uint32 base_shader_register,
			uint32 register_space)
		{
			m_num_resources = num_resources;
			m_type = type;
			m_shader_register = base_shader_register;
			m_register_space = register_space;
		}
		

		uint32 m_shader_register;
		uint32 m_register_space;

		uint32 m_num_resources;
		e_type m_type;
	};

	struct root_param_resource_table final
	{
		root_param_resource_table() = default;
		root_param_resource_table(const vector<root_param_resource_range>& ranges)
		{
			m_resource_ranges = ranges;
		}
		root_param_resource_table(const root_param_resource_range& range, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_common.m_register_space = space;
			m_common.m_shader_register = sh_reg;
			m_resource_ranges.push_back(range);
		}

		root_param_common m_common;
		vector<root_param_resource_range> m_resource_ranges{};
	};

	struct root_static_sampler final
	{
		root_static_sampler() = default;
		root_static_sampler(
			uint32 shader_register,
			uint32 register_space,
			e_shader_visibility visibility,
			float mip_load_bias = 0.0f,
			float min_lod = 0.0f,
			float max_lod = FLT_MAX,
			uint32 max_anisotropy = 16u,
			e_texture_wrap_mode wrap_u = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_v = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_w = e_texture_wrap_mode::wrap,
			e_filter filter = e_filter::anisotropic,
			e_comparison_func comp_func = e_comparison_func::lequal,
			e_border_color border_color = e_border_color::white)
			: m_mip_lod_bias{ mip_load_bias }
			, m_min_lod{ min_lod }
			, m_max_lod{ max_lod }
			, m_max_anisotropy{ max_anisotropy }
			, m_wrap_u{ wrap_u }
			, m_wrap_v{ wrap_v }
			, m_wrap_w{ wrap_w }
			, m_filter{ filter }
			, m_comparison_func{ comp_func }
			, m_border_color{ border_color }
		{
			m_common.m_visibility = visibility;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
		}

		root_param_common m_common;

		float m_mip_lod_bias = 0.0f;
		float m_min_lod = 0.0f;
		float m_max_lod = FLT_MAX;
		uint32 m_max_anisotropy = 16u;

		e_texture_wrap_mode m_wrap_u = e_texture_wrap_mode::wrap;
		e_texture_wrap_mode m_wrap_v = e_texture_wrap_mode::wrap;
		e_texture_wrap_mode m_wrap_w = e_texture_wrap_mode::wrap;

		e_filter m_filter = e_filter::anisotropic;
		e_comparison_func m_comparison_func = e_comparison_func::lequal;
		e_border_color m_border_color = e_border_color::white;
	};

	struct rootsignature_desc final
	{
		// internally adds to the root table
		inline void add_root_range(root_param_resource_range::e_type type, uint32 num_resources, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			root_param_resource_range range{ num_resources, type, sh_reg, space };

			root_param_resource_table table{};
			table.m_common.m_register_space = space;
			table.m_common.m_shader_register = sh_reg;
			table.m_common.m_visibility = vis;
			table.m_resource_ranges.push_back(range);

			m_resource_tables.push_back(table);
		}

		inline void add_root_resource(root_param_resource::e_type type, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_resources.push_back({ type, sh_reg, space, vis });
		}

		inline void add_root_constants(uint32 num_dwords, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_constants.push_back({ num_dwords, sh_reg, space, vis });
		}

		inline void add_root_sampler(
			uint32 shader_register,
			uint32 register_space,
			e_shader_visibility visibility,
			float mip_load_bias = 0.0f,
			float min_lod = 0.0f,
			float max_lod = FLT_MAX,
			uint32 max_anisotropy = 16u,
			e_texture_wrap_mode wrap_u = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_v = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_w = e_texture_wrap_mode::wrap,
			e_filter filter = e_filter::anisotropic,
			e_comparison_func comp_func = e_comparison_func::lequal,
			e_border_color border_color = e_border_color::white)
		{
			m_static_samplers.push_back({
				shader_register, 
				register_space, 
				visibility, 
				mip_load_bias, 
				min_lod, 
				max_lod,
				max_anisotropy,
				wrap_u,
				wrap_v,
				wrap_w,
				filter,
				comp_func,
				border_color});
		}

		vector<root_param_constants> m_constants;
		vector<root_param_resource> m_resources;
		vector<root_param_resource_table> m_resource_tables;
		vector<root_static_sampler> m_static_samplers;
	};

	class rootsignature : public base
	{
	public:
		
	protected:
		rootsignature(const rootsignature_desc& desc)
			: m_desc{desc}
		{

		}

	private:
		rootsignature_desc m_desc;
	};
}