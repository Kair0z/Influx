#pragma once

// influx::core
#include "core/shader.h"

// influx::graphics
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"
#include "influx_graphics/raytracing.h"

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

	enum class e_mesh_shader_slots : uint8
	{
		as, // amplification shader
		ms, // mesh shader
		ps, // pixel shader
		count
	};

	template <e_pipeline_type _t>
	class shader_slots
	{
	public:
		using enum_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			e_graphics_shader_slots,
			e_compute_shader_slots,
			e_raytracing_shader_slots,
			e_mesh_shader_slots>>;

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
			else if constexpr (_t == e_pipeline_type::mesh)
			{
				switch (type)
				{
					case e_mesh_shader_slots::as: return true;
					case e_mesh_shader_slots::ms: return false;
					case e_mesh_shader_slots::ps: return false;
					default: return false;
				}
			}
		}

		static constexpr bool is_optional(uint8 index)
		{
			return is_optional(static_cast<enum_type>(index));
		}

		inline void set(shader::e_shader_type type, const vector<byte>& shader_bytecode)
		{
			if constexpr (_t == e_pipeline_type::graphics)
			{
				switch (type)
				{
				case shader::e_shader_type::vs: set(e_graphics_shader_slots::vs, shader_bytecode); break;
				case shader::e_shader_type::ps: set(e_graphics_shader_slots::ps, shader_bytecode); break;
				case shader::e_shader_type::ds: set(e_graphics_shader_slots::ds, shader_bytecode); break;
				case shader::e_shader_type::gs: set(e_graphics_shader_slots::gs, shader_bytecode); break;
				case shader::e_shader_type::hs: set(e_graphics_shader_slots::hs, shader_bytecode); break;
				}
			}
			else if constexpr (_t == e_pipeline_type::compute)
			{
				switch (type)
				{
				case shader::e_shader_type::cs: set(e_compute_shader_slots::cs, shader_bytecode); break;
				}
			}
			else if constexpr (_t == e_pipeline_type::raytracing)
			{
				switch (type)
				{
				case shader::e_shader_type::rgs: set(e_raytracing_shader_slots::rgs, shader_bytecode); break;
				case shader::e_shader_type::mss: set(e_raytracing_shader_slots::mss, shader_bytecode); break;
				case shader::e_shader_type::chs: set(e_raytracing_shader_slots::chs, shader_bytecode); break;
				case shader::e_shader_type::ahs: set(e_raytracing_shader_slots::ahs, shader_bytecode); break;
				case shader::e_shader_type::ins: set(e_raytracing_shader_slots::ins, shader_bytecode); break;
				}
			}
			else if constexpr (_t == e_pipeline_type::mesh)
			{
				switch (type)
				{
				case shader::e_shader_type::as: set(e_mesh_shader_slots::as, shader_bytecode); break;
				case shader::e_shader_type::ms: set(e_mesh_shader_slots::ms, shader_bytecode); break;
				case shader::e_shader_type::ps: set(e_mesh_shader_slots::ps, shader_bytecode); break;
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

		inline const vector<byte>& get(uint8 idx) const
		{
			return m_shaders[idx];
		}

		static constexpr uint8 count = static_cast<uint8>(enum_type::count);
		static constexpr uint8 num = count;

	private:
		vector<byte> m_shaders[count]{};
	};

	using graphics_shaderslots = shader_slots<e_pipeline_type::graphics>;
	using compute_shaderslots = shader_slots<e_pipeline_type::compute>;
	using raytracing_shaderslots = shader_slots<e_pipeline_type::raytracing>;
	using mesh_shaderslots = shader_slots<e_pipeline_type::mesh>;

#pragma endregion

	// pipeline description
#pragma region pipelinedesc
	struct depth_stencil_desc final
	{
		inline static depth_stencil_desc default_no_stencil()
		{
			depth_stencil_desc desc{};
			desc.m_depth_enable = true;
			desc.m_stencil_enable = false;
			desc.m_depth_func = graphics::e_comparison_func::less;
			desc.m_format = graphics::e_format::d32;
			return desc;
		}

		bool m_depth_enable;
		bool m_stencil_enable;
		e_comparison_func m_depth_func;
		e_format m_format = e_format::d32;
	};

	struct rasterizer_desc final
	{
		inline static rasterizer_desc default_graphics()
		{
			rasterizer_desc desc{};
			desc.m_cullmode = graphics::e_cull_mode::nocull;
			desc.m_fillmode = graphics::e_fill_mode::solid;
			desc.m_front_ccw = false;
			desc.m_depth_clip_enable = false;
			desc.m_multisample = false;
			desc.m_antialiased_line = false;
			desc.m_conservative = false;
			desc.m_depth_bias = 0;
			desc.m_depth_bias_clamp = 0.0f;
			desc.m_slope_depth_bias = 0.0f;
			desc.m_forced_samplecount = 0u;
			return desc;
		}

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

	struct blend_desc final
	{
		inline static blend_desc default_write_all()
		{
			blend_desc desc{};
			desc.m_enabled = false;
			desc.m_src;
			desc.m_dest;
			desc.m_op;
			desc.m_srcalpha;
			desc.m_destalpha;
			desc.m_op_alpha;
			desc.m_write_mask = 15u; // all
			return desc;
		}

		bool m_enabled = false;
		e_blend m_src;
		e_blend m_dest;
		e_blendop m_op;
		e_blend m_srcalpha;
		e_blend m_destalpha;
		e_blendop m_op_alpha;
		uint8 m_write_mask = 15u; // all
	};

	struct graphics_pipeline_desc final
	{
		// shaders
		shader_slots<e_pipeline_type::graphics> m_shaders{};

		// misc
		uint32 m_sample_mask = (uint32)-1;
		uint32 m_sample_count = 1u;
		inline graphics_pipeline_desc& set_sample_desc(uint32 sample_count, uint32 sample_mask = -1)
		{
			m_sample_mask = sample_mask;
			m_sample_count = sample_count;
			return *this;
		}

		e_primitive_topology_type m_prim_type = e_primitive_topology_type::triangle;

		// depth / stencil
		depth_stencil_desc m_depth_stencil;

		// rasterizer
		rasterizer_desc m_rasterizer;

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

		inline graphics_pipeline_desc& set_rendertarget_desc(uint8 index, bool enabled, e_format format)
		{
			influx_assert(index < k_max_render_targets);

			m_rtvs[index].m_enabled = enabled;
			m_rtvs[index].m_format = format;
			return *this;
		}

		// blends
		bool m_blend_alpha_to_coverage_enabled = false;
		blend_desc m_blends[k_max_render_targets]{};
		inline graphics_pipeline_desc& set_blend_desc(uint8 index, const blend_desc& desc)
		{
			influx_assert(index < k_max_render_targets);
			m_blends[index] = desc;
			return *this;
		}
	};

	struct compute_pipeline_desc final
	{
		shader_slots<e_pipeline_type::compute> m_shaders{};
	};

	struct raytracing_pipeline_desc final
	{
		uint32 m_max_recursion_depth = 8u;
		shader_slots<e_pipeline_type::raytracing> m_shaders{};
		vector<string> m_shader_export_names{};
		vector<hitgroup> m_hitgroups{};
	};

	struct mesh_pipeline_desc final
	{
		shader_slots<e_pipeline_type::mesh> m_shaders{};

		uint32 m_sample_mask = (uint32)-1;
		uint32 m_sample_count = 1u;
		inline mesh_pipeline_desc& set_sample_desc(uint32 sample_count, uint32 sample_mask = -1)
		{
			m_sample_mask = sample_mask;
			m_sample_count = sample_count;
			return *this;
		}

		e_primitive_topology_type m_prim_type = e_primitive_topology_type::triangle;

		// depth / stencil
		depth_stencil_desc m_depth_stencil;

		// rasterizer
		rasterizer_desc m_rasterizer;

		// RTVs
		struct rtv_desc
		{
			bool m_enabled = false;
			e_format m_format = e_format::rgba8;
		};
		rtv_desc m_rtvs[k_max_render_targets]{};
		
		inline mesh_pipeline_desc& set_rendertarget_desc(uint8 index, bool enabled, e_format format)
		{
			influx_assert(index < k_max_render_targets);

			m_rtvs[index].m_enabled = enabled;
			m_rtvs[index].m_format = format;
			return *this;
		}

		// blends
		bool m_blend_alpha_to_coverage_enabled = false;
		blend_desc m_blends[k_max_render_targets]{};

		inline mesh_pipeline_desc& set_blend_desc(uint8 index, const blend_desc& desc)
		{
			influx_assert(index < k_max_render_targets);
			m_blends[index] = desc;
			return *this;
		}
	};

	template <e_pipeline_type _t>
	using pipeline_desc = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
		graphics::graphics_pipeline_desc,
		graphics::compute_pipeline_desc,
		graphics::raytracing_pipeline_desc,
		graphics::mesh_pipeline_desc>>;

#pragma endregion

	namespace detail
	{
		class base_pipeline : public graphics::base 
		{
		public:
			virtual e_pipeline_type get_type() const = 0;
		};
	}

	template <e_pipeline_type _t>
	class pipeline : public detail::base_pipeline
	{
	public:
		using desc_type = pipeline_desc<_t>;

		inline virtual e_pipeline_type get_type() const override final
		{
			return k_type;
		}

	protected:
		static constexpr e_pipeline_type k_type = _t;
		pipeline(const desc_type& desc) : m_desc{ desc } {}
		desc_type m_desc{};
	};

	using graphics_pipeline		= pipeline<e_pipeline_type::graphics>;
	using compute_pipeline		= pipeline<e_pipeline_type::compute>;
	using raytracing_pipeline	= pipeline<e_pipeline_type::raytracing>;
	using mesh_pipeline			= pipeline<e_pipeline_type::mesh>;
}