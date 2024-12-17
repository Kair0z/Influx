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
		string m_vs_name = "";
		string m_ps_name = "";

		// rasterizer
		uint32 m_primitive_type			= 0u; // triangle
		uint32 m_cullmode				= 2u;
		uint32 m_fillmode				= 1u;
		uint32 m_forced_samplecount		= 0u;
		uint32 m_sample_mask			= (uint32)-1;
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
		uint32 m_depth_format			= 5u; // d32

		// rtvs & dsvs
		static constexpr uint8 k_max_num_rendertargets = 8u;
		bool m_rtv_actives[k_max_num_rendertargets] = { true, false, false, false, false, false, false, false };
		uint8 m_rtv_formats[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };

		// rtv blend
		bool m_blend_actives[k_max_num_rendertargets] = { false, false, false, false, false, false, false, false };
		uint32 m_blend_sources[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_blend_dests[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_blend_ops[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_sources[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_dests[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint32 m_alpha_ops[k_max_num_rendertargets] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
		uint8 m_blend_writemasks[k_max_num_rendertargets] = { 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u };
	};

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
		

	private:
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