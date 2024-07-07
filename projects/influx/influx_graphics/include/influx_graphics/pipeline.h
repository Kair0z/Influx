#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

namespace influx::graphics
{
	struct pipeline_desc final
	{
		// shaders
		vector<byte> m_vs;
		vector<byte> m_ps;

		// depth / stencil
		struct
		{
			bool m_depth_enable;
			bool m_stencil_enable;
			e_comparison_func m_depth_func;
			// depth write mask
		} m_depth_stencil_desc;

		// input layout
		// ...
		
		// root signature
		// ...
		
		// misc
		uint32 m_sample_mask = (uint32)-1;
		uint32 m_sample_count = 1u;
		e_primitive_topology_type m_prim_type = e_primitive_topology_type::triangle;

		// RTVs
		struct
		{
			bool m_enabled = false;
			e_format m_format = e_format::rgba8;

		} m_rtvs[k_max_render_targets];

		// DSV
		e_format m_format_dsv;
	};

	class pipeline : public base
	{
	public:
		
	protected:
		pipeline(const pipeline_desc& desc)
			: m_desc{ desc }
		{

		}

	private:
		pipeline_desc m_desc;
	};
}