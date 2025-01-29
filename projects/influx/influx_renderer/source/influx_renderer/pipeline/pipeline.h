#pragma once
#include "influx_renderer.h"

// influx::core
#include "core/string.h"
#include "core/container/map.h"

// influx::graphics
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/commandlist.h"

#pragma region graphics declarations
namespace influx::graphics
{
	class device;
	class rootsignature;
	class commandlist;
	struct descriptor_range;
}
#pragma endregion

namespace influx::renderer
{
	struct graphics_pipeline_signature final
	{
#pragma region enums
		enum cullmode : uint32
		{
			front	= 0,
			back	= 1,
			none	= 2
		};
		enum primitive_type : uint32
		{
			triangle,
			line
		};
		enum fillmode : uint32
		{
			solid		= 0,
			wireframe	= 1
		};
		enum samplemask : uint32
		{
			all = (uint32)-1
		};
		enum blendmask : uint32
		{
			blend_all = 15u
		};
		enum format : uint32
		{
			rgba8	= 0u,
			default_color = rgba8,
			r32		= 1u,
			rg32	= 2u,
			rgb32	= 3u,
			rgba32	= 4u,
			d32		= 5u,
			default_depth = d32,
			u16		= 6u,
			u32		= 7u
		};
		enum blendop : uint32
		{
			op_add				= 1,
			op_subtract			= 2,
			op_rev_subtract		= 3,
			op_min				= 4,
			op_max				= 5
		};
		enum blend : uint32
		{
			bl_zero				= 1,
			bl_one				= 2,
			bl_src_color		= 3,
			bl_inv_src_color	= 4,
			bl_src_alpha		= 5,
			bl_inv_src_alpha	= 6,
			bl_dest_alpha		= 7,
			bl_inv_dest_alpha	= 8,
			bl_dest_color		= 9,
			bl_inv_dest_color	= 10,
			bl_src_alpha_sat	= 11,
			bl_blend_factor		= 14,
			bl_inv_blend_factor = 15,
			bl_src1_color		= 16,
			bl_inv_src1_color	= 17,
			bl_src1_alpha		= 18,
			bl_inv_src1_alpha	= 19,
			bl_alpha_factor		= 20,
			bl_inv_alpha_factor = 21,
		};
#pragma endregion
		bool m_bindless = false;
		string m_vs_name = "";
		string m_ps_name = "";

		// rasterizer
		uint32 m_primitive_type			= primitive_type::triangle;
		uint32 m_cullmode				= cullmode::back;
		uint32 m_fillmode				= fillmode::solid;
		uint32 m_forced_samplecount		= 0u;
		uint32 m_sample_mask			= samplemask::all;
		uint32 m_sample_count			= 1u;
		bool m_front_ccw				= true;
		bool m_depthclip				= true;
		bool m_multisample				= false;
		bool m_antialiased_line			= false;
		bool m_conservative_raster		= false;
		int m_depthbias					= 0;
		float m_depthbias_clamp			= 0.0f;
		float m_slope_depthbias			= 0.0f;

		// depth / stencil
		bool m_depth_enable				= false;
		bool m_stencil_enable			= false;
		uint32 m_depth_comparison		= 0u;
		uint32 m_depth_format			= format::d32;

		// rtvs & dsvs
		static constexpr uint8 k_max_num_rendertargets = 8u;
		bool m_rtv_actives[k_max_num_rendertargets]		= { true, false, false, false, false, false, false, false };
		uint8 m_rtv_formats[k_max_num_rendertargets]	= { format::rgba8, 0u, 0u, 0u, 0u, 0u, 0u, 0u };

		// rtv blend
		bool m_blend_actives[k_max_num_rendertargets]		= { false, false, false, false, false, false, false, false };
		uint32 m_blend_sources[k_max_num_rendertargets]		= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_blend_dests[k_max_num_rendertargets]		= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_blend_ops[k_max_num_rendertargets]			= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_sources[k_max_num_rendertargets]		= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_dests[k_max_num_rendertargets]		= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_ops[k_max_num_rendertargets]			= { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint8 m_blend_writemasks[k_max_num_rendertargets]	= { blendmask::blend_all, 0u, 0u, 0u, 0u, 0u, 0u, 0u };

		bool operator==(const graphics_pipeline_signature&) const = default; // Automatically generates an equality operator
	};

	struct compute_pipeline_signature final
	{
		bool m_bindless = false;
		string m_cs_name = "";
	};

	struct raytracing_pipeline_signature final
	{

	};

	template <graphics::e_pipeline_type _t>
	using pipeline_signature = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
		graphics_pipeline_signature,
		compute_pipeline_signature,
		raytracing_pipeline_signature>>;

	namespace detail
	{
		class pipeline
		{
		public:
			graphics::graphics_pipeline* create_graphics(
				graphics::device& device, 
				const graphics_pipeline_signature& signature,
				const shader_data& vs,
				const shader_data& ps);

			graphics::compute_pipeline* create_compute(
				graphics::device& device, 
				const compute_pipeline_signature& signature,
				const shader_data& cs);

			graphics::raytracing_pipeline* create_raytracing(graphics::device& device, const raytracing_pipeline_signature& signature);

			virtual graphics::e_pipeline_type get_type() const = 0;

			graphics::rootsignature_desc m_rootsig_desc{};
			graphics::rootsignature* m_rootsig = nullptr;
			umap<string, uint32> m_name_to_register;
			umap<string, uint32> m_name_to_param_idx;
		};

		template <graphics::e_pipeline_type _t>
		class tpipeline : public pipeline
		{
			using signature_type = pipeline_signature<_t>;
			using pipeline_desc_type = graphics::pipeline_desc<_t>;
			using pipeline_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
				graphics::graphics_pipeline,
				graphics::compute_pipeline,
				graphics::raytracing_pipeline>>;

			signature_type m_signature;
			pipeline_type* m_pipeline = nullptr;
			pipeline_desc_type m_desc{};
			
			debug_name m_name;

		public:
			tpipeline(graphics::device& device, const signature_type& signature)
				: pipeline()
				, m_signature{ signature }
			{
				pipeline_desc_type desc{};

				if constexpr (_t == graphics::e_pipeline_type::graphics)
				{
					m_pipeline = create_graphics(device, desc,);
				}
				else if constexpr (_t == graphics::e_pipeline_type::compute)
				{
					m_pipeline = create_compute(device, desc);
				}
				else if constexpr (_t == graphics::e_pipeline_type::raytracing)
				{
					m_pipeline = create_raytracing(device, desc);
				}
			}

			void set_state(graphics::commandlist& commandlist)
			{
				commandlist.set(m_rootsig);
				commandlist.set(m_pipeline);
			}

			template <typename _constants>
			void set_constants(graphics::commandlist& cmdlist, const string& name, _constants& constants)
			{
				set_constants(cmdlist, name, sizeof(_constants) / sizeof(uint32), &constants);
			}

			void set_constants(graphics::commandlist& cmdlist, const string& name, uint32 num_dwords, void* data)
			{
				cmdlist.set_constants(get_param_index(name), num_dwords, data);
			}

			void set_resource_table(graphics::commandlist& cmdlist, const string& name, const graphics::descriptor_range& gpu_range)
			{
				cmdlist.set(gpu_range, get_param_index(name));
			}

			uint32 get_shader_register(const string& resource_name)
			{
				return m_name_to_register[resource_name];
			}

			uint32 get_param_index(const string& resource_name)
			{
				return m_name_to_param_idx[resource_name];
			}

			const debug_name& get_name() const
			{
				return m_pipeline->get_name();
			}

			void set_name(const debug_name& name)
			{
				m_pipeline->set_name(name);
			}

			const signature_type& get_signature() const
			{
				return m_signature;
			}

			// todo
			static tpipeline* load_from_file(const string& path)
			{
				return nullptr;
			}
			void save_to_file(const string& path) const
			{

			}

			virtual graphics::e_pipeline_type get_type() const override { return _t; }
		};
	}
	
	using graphics_pipeline = detail::tpipeline<graphics::e_pipeline_type::graphics>;
	using compute_pipeline = detail::tpipeline<graphics::e_pipeline_type::compute>;
	using raytracing_pipeline = detail::tpipeline<graphics::e_pipeline_type::raytracing>;
}