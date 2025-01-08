#pragma once

#include "influx_renderer.h"

// influx::core
#include "core/string.h"
#include "core/container/map.h"

// influx::graphics
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"

#pragma region graphics declarations
namespace influx::graphics
{
	class device;
	class pipeline;
	class rootsignature;
	class commandlist;
	struct descriptor_range;
	struct pipeline_desc;
}
#pragma endregion

namespace influx::renderer
{
	struct pipeline_signature final
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

		string m_vs_name = "";
		string m_ps_name = "";

		// rasterizer
		uint32 m_primitive_type			= primitive_type::triangle;
		uint32 m_cullmode				= cullmode::back;
		uint32 m_fillmode				= fillmode::solid;
		uint32 m_forced_samplecount		= 0u;
		uint32 m_sample_mask			= samplemask::all;
		uint32 m_sample_count			= 1u;
		bool m_front_ccw				= false;
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
	};

	inline static bool operator==(const pipeline_signature& a, const pipeline_signature& b)
	{
		const bool settings_match = (
			a.m_vs_name == b.m_vs_name &&
			a.m_ps_name == b.m_ps_name &&
			a.m_primitive_type == b.m_primitive_type &&
			a.m_cullmode == b.m_cullmode &&
			a.m_fillmode == b.m_fillmode &&
			a.m_forced_samplecount == b.m_forced_samplecount &&
			a.m_sample_mask == b.m_sample_mask &&
			a.m_sample_count == b.m_sample_count &&
			a.m_front_ccw == b.m_front_ccw &&
			a.m_depthclip == b.m_depthclip &&
			a.m_multisample == b.m_multisample &&
			a.m_antialiased_line == b.m_antialiased_line &&
			a.m_conservative_raster == b.m_conservative_raster &&
			a.m_depthbias == b.m_depthbias &&
			a.m_depthbias_clamp == b.m_depthbias_clamp &&
			a.m_slope_depthbias == b.m_slope_depthbias &&
			a.m_depth_enable == b.m_depth_enable &&
			a.m_stencil_enable == b.m_stencil_enable &&
			a.m_depth_comparison == b.m_depth_comparison &&
			a.m_depth_format == b.m_depth_format);

		bool render_target_settings_match = true;
		for (uint8 i = 0u; i < pipeline_signature::k_max_num_rendertargets; ++i)
		{
			render_target_settings_match &=
				a.m_blend_actives[i] == b.m_blend_actives[i] &&
				a.m_blend_sources[i] == b.m_blend_sources[i] &&
				a.m_blend_dests[i] == b.m_blend_dests[i] &&
				a.m_blend_ops[i] == b.m_blend_ops[i] &&
				a.m_alpha_sources[i] == b.m_alpha_sources[i] &&
				a.m_alpha_dests[i] == b.m_alpha_dests[i] &&
				a.m_alpha_ops[i] == b.m_alpha_ops[i] &&
				a.m_blend_writemasks[i] == b.m_blend_writemasks[i];
		}

		return settings_match;
	}

	inline static bool operator!=(const pipeline_signature& a, const pipeline_signature& b)
	{
		return !(a == b);
	}

	class pipeline final
	{
	public:
		pipeline(
			graphics::device* device,
			const pipeline_signature& signature,
			renderer::shader_data const* vs,
			renderer::shader_data const* ps);

		static pipeline* load_from_file(const string& path);

		void set_state(graphics::commandlist* cmdlist);

		template <typename _constants>
		void set_constants(graphics::commandlist* cmdlist, const string& name, _constants& constants)
		{
			set_constants(cmdlist, name, sizeof(_constants) / sizeof(uint32), &constants);
		}

		void set_constants(graphics::commandlist* cmdlist, const string& name, uint32 num_dwords, void* data);

		void set_resource_table(graphics::commandlist* cmdlist, const string& name, const graphics::descriptor_range& gpu_range);

		uint32 get_shader_register(const string& resource_name);
		uint32 get_param_index(const string& resource_name);

#if INFLUX_DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

		void save_to_file(const string& path) const;
		
		const pipeline_signature& get_signature() const;

	private:
		pipeline_signature m_signature;
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::pipeline* mp_pipeline = nullptr;
		umap<string, uint32> m_name_to_register;
		umap<string, uint32> m_name_to_param_idx;
		graphics::pipeline_desc m_create_desc{};
		graphics::rootsignature_desc m_rootsig_desc{};

#if INFLUX_DEBUG
		string m_debug_name;
#endif
	};
}