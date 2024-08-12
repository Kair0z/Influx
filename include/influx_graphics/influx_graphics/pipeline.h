#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

namespace influx::graphics
{
	struct pipeline_input_element final
	{
		string m_semantic_name;
		uint32 m_semantic_idx;
		e_format m_format;
		uint32 m_input_slot;
		uint32 m_aligned_byteoffset;
		
		bool m_is_per_instance; // if not, per vertex
		uint32 m_instance_data_steprate;
	};

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
		} m_depth_stencil;

		// rasterizer
		struct
		{
			e_cull_mode m_cullmode;
		} m_rasterizer;

		// input layout
		vector<pipeline_input_element> m_input_elements{};

		inline void add_input_element(
			const string& semantic_name,
			uint32 semantic_index,
			e_format format,
			uint32 input_slot,
			bool is_per_instance,
			uint32 instance_steprate)
		{
			pipeline_input_element new_element{};
			new_element.m_format = format;
			new_element.m_input_slot = input_slot;
			new_element.m_instance_data_steprate = instance_steprate;
			new_element.m_is_per_instance = is_per_instance;
			new_element.m_semantic_name = semantic_name;
			new_element.m_semantic_idx = semantic_index;

			// deduce byteoffset
			if (!m_input_elements.empty())
			{
				const pipeline_input_element& last_element = m_input_elements.back();
				new_element.m_aligned_byteoffset = 
					last_element.m_aligned_byteoffset + (uint32)deduce_bytesize(last_element.m_format);
			}
			
			m_input_elements.push_back(new_element);
		}
		
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