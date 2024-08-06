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

	struct root_param_common final
	{
		e_shader_visibility m_visibility;
		uint32 m_shader_register;
		uint32 m_register_space;
	};

	struct root_param_constants final
	{
		root_param_common m_common;
		uint32 m_num_dwords;
	};

	struct root_param_resource final
	{
		root_param_common m_common;
	};

	struct root_param_resource_range final
	{
		enum class e_type
		{
			srv,
			cbv,
			uav,
			sampler,
			count
		};

		uint32 m_shader_register;
		uint32 m_register_space;

		uint32 m_num_resources;
		e_type m_type;
	};

	struct root_param_resource_table final
	{
		root_param_common m_common;
		vector<root_param_resource_range> m_resource_ranges{};
	};

	struct root_static_sampler final
	{
		root_param_common m_common;

		float m_mip_lod_bias;
		float m_min_lod;
		float m_max_lod;
		uint32 m_max_anisotropy;

		e_texture_wrap_mode m_wrap_u;
		e_texture_wrap_mode m_wrap_v;
		e_texture_wrap_mode m_wrap_w;

		e_filter m_filter;
		e_comparison_func m_comparison_func;
		e_border_color m_border_color;
		// D3D12_STATIC_BORDER_COLOR BorderColor;
	};

	struct rootsignature_desc final
	{
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