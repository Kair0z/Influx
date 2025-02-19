#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

namespace influx::graphics
{
	// shader slots per type of pipeline
#pragma region shaderslots
	enum class e_graphics_shader_slots : uint8
	{
		vs,
		ps,
		ds,
		gs,
		hs,
		count
	};
	enum class e_compute_shader_slots : uint8
	{
		cs,
		count
	};
	enum class e_raytracing_shader_slots : uint8
	{
		rgs,
		mss,
		chs,
		ahs,
		ins,
		count
	};

	template <e_pipeline_type _t>
	class shader_slots
	{
	public:
		using enum_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			e_graphics_shader_slots,
			e_compute_shader_slots,
			e_raytracing_shader_slots>>;

		static constexpr bool is_optional(const enum_type type)
		{
			if constexpr (_t == e_pipeline_type::graphics)
			{
				switch (type)
				{
				case e_graphics_shader_slots::vs: return false;
				case e_graphics_shader_slots::ps: return true;
				case e_graphics_shader_slots::ds: return true;
				case e_graphics_shader_slots::gs: return true;
				case e_graphics_shader_slots::hs: return true;
				default: return false;
				}
			}
			else if constexpr (_t == e_pipeline_type::compute)
			{
				switch (type)
				{
				case e_compute_shader_slots::cs: return false;
				default: return false;
				}
			}
			else if constexpr (_t == e_pipeline_type::raytracing)
			{
				switch (type)
				{
				case e_raytracing_shader_slots::rgs: return false;
				case e_raytracing_shader_slots::mss: return true;
				case e_raytracing_shader_slots::chs: return true;
				case e_raytracing_shader_slots::ahs: return true;
				case e_raytracing_shader_slots::ins: return true;
				default: return false;
				}
			}
		}

		inline void set(enum_type slot, const vector<byte>& shader_bytecode)
		{
			m_shaders[static_cast<uint8>(slot)] = shader_bytecode;
		}

		inline const vector<byte>& get(enum_type slot) const
		{
			return m_shaders[static_cast<uint8>(slot)];
		}

		static constexpr uint8 count = static_cast<uint8>(enum_type::count);
		static constexpr uint8 num = count;

	private:
		vector<byte> m_shaders[count]{};
	};

	using graphics_shaderslots = shader_slots<e_pipeline_type::graphics>;
	using compute_shaderslots = shader_slots<e_pipeline_type::compute>;
	using raytracing_shaderslots = shader_slots<e_pipeline_type::raytracing>;
#pragma endregion

	// pipeline description
#pragma region pipelinedesc
	struct graphics_pipeline_desc final
	{
		// shaders
		shader_slots<e_pipeline_type::graphics> m_shaders{};

		// misc
		uint32 m_sample_mask = (uint32)-1;
		uint32 m_sample_count = 1u;
		e_primitive_topology_type m_prim_type = e_primitive_topology_type::triangle;

		// DSV
		e_format m_format_dsv = e_format::d32;

		// depth / stencil
		struct depth_stencil final
		{
			bool m_depth_enable;
			bool m_stencil_enable;
			e_comparison_func m_depth_func;
			// depth write mask
		};
		depth_stencil m_depth_stencil;

		// rasterizer
		struct rasterizer final
		{
			e_cull_mode m_cullmode = e_cull_mode::back;
			e_fill_mode m_fillmode = e_fill_mode::solid;
			bool m_front_ccw = false;
			int m_depth_bias = 0;
			float m_depth_bias_clamp = 0.0f;
			float m_slope_depth_bias = 0.0f;
			bool m_depth_clip_enable = true;
			bool m_multisample = false;
			bool m_antialiased_line = false;
			uint32 m_forced_samplecount = 0u;
			bool m_conservative = false;
		};
		rasterizer m_rasterizer;

		// input layout
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

		// RTVs
		struct rtv_desc
		{
			bool m_enabled = false;
			e_format m_format = e_format::rgba8;
		};
		rtv_desc m_rtvs[k_max_render_targets]{};

		// blends
		struct blend_desc final
		{
			bool m_enabled = false;
			e_blend m_src;
			e_blend m_dest;
			e_blendop m_op;
			e_blend m_srcalpha;
			e_blend m_destalpha;
			e_blendop m_op_alpha;
			uint8 m_write_mask = 15u; // all
		};
		blend_desc m_blends[k_max_render_targets]{};
		bool m_blend_alpha_to_coverage_enabled = false;
	};

	struct compute_pipeline_desc final
	{
		shader_slots<e_pipeline_type::compute> m_shaders{};
	};

	struct raytracing_pipeline_desc final
	{
		shader_slots<e_pipeline_type::raytracing> m_shaders{};
	};

	template <e_pipeline_type _t>
	using pipeline_desc = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
		graphics::graphics_pipeline_desc,
		graphics::compute_pipeline_desc,
		graphics::raytracing_pipeline_desc>>;
#pragma endregion

	namespace detail
	{
		class base_pipeline : public graphics::base {};
	}

	template <e_pipeline_type _t>
	class pipeline : public detail::base_pipeline
	{
	public:
		using desc_type = pipeline_desc<_t>;

	protected:
		static constexpr e_pipeline_type k_type = _t;
		pipeline(const desc_type& desc) : m_desc{ desc } {}
		desc_type m_desc{};
	};

	using graphics_pipeline		= pipeline<e_pipeline_type::graphics>;
	using compute_pipeline		= pipeline<e_pipeline_type::compute>;
	using raytracing_pipeline	= pipeline<e_pipeline_type::raytracing>;
}