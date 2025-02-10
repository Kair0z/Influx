#pragma once
#include "influx_renderer.h"

// influx::core
#include "core/string.h"
#include "core/container/map.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/shader_manager.h"

// influx::graphics
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

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
#pragma region shaderslots
	template <graphics::e_pipeline_type _t>
	static constexpr graphics::shader_slots<_t>::enum_type type_to_slot(shader::e_shader_type type)
	{
		if constexpr (_t == graphics::e_pipeline_type::graphics)
		{
			switch (type)
			{
			case shader::e_shader_type::vs: return graphics::e_graphics_shader_slots::vs;
			case shader::e_shader_type::ps: return graphics::e_graphics_shader_slots::ps;
			case shader::e_shader_type::gs: return graphics::e_graphics_shader_slots::gs;
			case shader::e_shader_type::ds: return graphics::e_graphics_shader_slots::ds;
			case shader::e_shader_type::hs: return graphics::e_graphics_shader_slots::hs;
			default: return graphics::e_graphics_shader_slots::count;
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::compute)
		{
			switch (type)
			{
			case shader::e_shader_type::cs: return graphics::e_compute_shader_slots::cs;
			default: return graphics::e_compute_shader_slots::count;
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::raytracing)
		{
			switch (type)
			{
			case shader::e_shader_type::rgs: return graphics::e_raytracing_shader_slots::rgs;
			case shader::e_shader_type::mss: return graphics::e_raytracing_shader_slots::mss;
			case shader::e_shader_type::chs: return graphics::e_raytracing_shader_slots::chs;
			case shader::e_shader_type::ahs: return graphics::e_raytracing_shader_slots::ahs;
			case shader::e_shader_type::ins: return graphics::e_raytracing_shader_slots::ins;
			default: return graphics::e_raytracing_shader_slots::count;
			}
		}
		else
		{
			static_assert(false, "unsupported pipeline type");
		}
	}

	template <graphics::e_pipeline_type _t>
	static constexpr shader::e_shader_type slot_to_type(typename graphics::shader_slots<_t>::enum_type slot)
	{
		if constexpr (_t == graphics::e_pipeline_type::graphics)
		{
			switch (slot)
			{
			case graphics::e_graphics_shader_slots::vs: return shader::e_shader_type::vs;
			case graphics::e_graphics_shader_slots::ps: return shader::e_shader_type::ps;
			case graphics::e_graphics_shader_slots::gs: return shader::e_shader_type::gs;
			case graphics::e_graphics_shader_slots::ds: return shader::e_shader_type::ds;
			case graphics::e_graphics_shader_slots::hs: return shader::e_shader_type::hs;
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::compute)
		{
			switch (slot)
			{
			case graphics::compute_shaderslots::cs: return shader::e_shader_type::cs;
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::raytracing)
		{
		}
		else
		{
			static_assert(false, "unsupported pipeline type");
		}
	}
#pragma endregion

#pragma region pipeline signatures
	struct graphics_pipeline_signature final
	{
		inline uint64 get_hash() const
		{
			uint64 result_hash = 0u;
			std::hash<byte> hasher{};
			std::hash<string> string_hasher{};

			// let's be lazy here :)))
			byte const* raw_data = reinterpret_cast<byte const*>(this);
			for (uint64 i = 0u; i < sizeof(graphics_pipeline_signature); ++i)
			{
				const byte current_byte = raw_data[i];
				result_hash = result_hash ^ (hasher(current_byte) + 0x9e3779b9 + (result_hash << 6) + (result_hash >> 2));
			}

			for (uint32 i = 0u; i < graphics::graphics_shaderslots::num; ++i)
			{
				result_hash ^= string_hasher(m_shader_identifiers[i]);
			}
			
			return result_hash;
		}

		// rtvs & dsvs
		static constexpr uint8 k_max_num_rendertargets = graphics::k_max_render_targets;

#pragma region enums
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
			u32		= 7u,
			u32_4	= 8u,
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

		// shaders
		string m_shader_identifiers[ graphics::graphics_shaderslots::num ]{};

		// rasterizer
		graphics::e_primitive_topology_type m_primitive_type		= graphics::e_primitive_topology_type::triangle;
		graphics::e_cull_mode				m_cullmode				= graphics::e_cull_mode::back;
		graphics::e_fill_mode				m_fillmode				= graphics::e_fill_mode::solid;
		uint32								m_forced_samplecount	= 0u;
		uint32								m_sample_mask			= samplemask::all;
		uint32								m_sample_count			= 1u;
		bool								m_front_ccw				= true;
		bool								m_depthclip				= true;
		bool								m_multisample			= false;
		bool								m_antialiased_line		= false;
		bool								m_conservative_raster	= false;
		int									m_depthbias				= 0;
		float								m_depthbias_clamp		= 0.0f;
		float								m_slope_depthbias		= 0.0f;

		// depth target
		bool						m_depth_enable		= false;
		bool						m_stencil_enable	= false;
		graphics::e_comparison_func	m_depth_comparison	= graphics::e_comparison_func::less;
		graphics::e_format			m_depth_format		= graphics::e_format::d32;

		// render targets
		bool				m_rtv_actives[graphics::k_max_render_targets]	= { true };
		graphics::e_format	m_rtv_formats[graphics::k_max_render_targets]	= { graphics::e_format::rgba8 };

		// blend
		bool m_blend_actives				[graphics::k_max_render_targets] = { false };
		graphics::e_blend	m_blend_sources	[graphics::k_max_render_targets] = { graphics::e_blend::one };
		graphics::e_blend	m_blend_dests	[graphics::k_max_render_targets] = { graphics::e_blend::zero };
		graphics::e_blendop m_blend_ops		[graphics::k_max_render_targets] = { graphics::e_blendop::add };
		graphics::e_blend	m_alpha_sources	[graphics::k_max_render_targets] = { graphics::e_blend::one };
		graphics::e_blend	m_alpha_dests	[graphics::k_max_render_targets] = { graphics::e_blend::zero };
		graphics::e_blendop m_alpha_ops		[graphics::k_max_render_targets] = { graphics::e_blendop::add };
		blendmask m_blend_writemasks		[graphics::k_max_render_targets] = { blendmask::blend_all };

		bool operator==(const graphics_pipeline_signature&) const = default; // Automatically generates an equality operator
		
		void set_shader_id(graphics::e_graphics_shader_slots slot, const string& identifier)
		{
			m_shader_identifiers[static_cast<uint8>(slot)] = identifier;
		}

		string get_shader_id(graphics::e_graphics_shader_slots slot) const
		{
			return m_shader_identifiers[static_cast<uint8>(slot)];
		}

		uint32 get_num_active_rtvs() const
		{
			uint32 count = 0u;
			for (uint8 i = 0u; i < k_max_num_rendertargets; ++i)
			{
				count += m_rtv_actives[i] ? 1u : 0u;
			}
			return count;
		}

		bool is_valid() const
		{
			// any graphics pipeline requires a valid vs!
			const bool vs_valid = get_shader_id(graphics::e_graphics_shader_slots::vs).empty() == false;
			const bool rtvs_valid = get_num_active_rtvs() > 0u;

			return vs_valid && rtvs_valid;
		}
	};

	struct compute_pipeline_signature final
	{
		inline uint64 get_hash() const
		{
			uint64 result_hash = 0u;
			std::hash<byte> hasher{};
			std::hash<string> string_hasher{};

			// let's be lazy here :)))
			byte const* raw_data = reinterpret_cast<byte const*>(this);
			for (uint64 i = 0u; i < sizeof(graphics_pipeline_signature); ++i)
			{
				const byte current_byte = raw_data[i];
				result_hash = result_hash ^ (hasher(current_byte) + 0x9e3779b9 + (result_hash << 6) + (result_hash >> 2));
			}

			for (uint32 i = 0u; i < graphics::compute_shaderslots::num; ++i)
			{
				result_hash ^= string_hasher(m_shader_identifiers[i]);
			}

			return result_hash;
		}

		void set_shader_id(graphics::e_compute_shader_slots slot, const string& identifier)
		{
			m_shader_identifiers[static_cast<uint8>(slot)] = identifier;
		}

		const string& get_shader_id(graphics::e_compute_shader_slots slot) const
		{
			return m_shader_identifiers[static_cast<uint8>(slot)];
		}

		bool m_bindless = false;
		string m_shader_identifiers[graphics::compute_shaderslots::num]{};
		bool operator==(const compute_pipeline_signature&) const = default; // Automatically generates an equality operator
		
		bool is_valid() const
		{
			return true;
		}
	};

	struct raytracing_pipeline_signature final
	{
		inline uint64 get_hash() const
		{
			uint64 result_hash = 0u;
			std::hash<byte> hasher{};
			std::hash<string> string_hasher{};

			// let's be lazy here :)))
			byte const* raw_data = reinterpret_cast<byte const*>(this);
			for (uint64 i = 0u; i < sizeof(graphics_pipeline_signature); ++i)
			{
				const byte current_byte = raw_data[i];
				result_hash = result_hash ^ (hasher(current_byte) + 0x9e3779b9 + (result_hash << 6) + (result_hash >> 2));
			}

			for (uint32 i = 0u; i < graphics::raytracing_shaderslots::num; ++i)
			{
				result_hash ^= string_hasher(m_shader_identifiers[i]);
			}

			return result_hash;
		}

		bool m_bindless = false;
		string m_shader_identifiers[graphics::raytracing_shaderslots::num]{};
		bool operator==(const raytracing_pipeline_signature&) const = default; // Automatically generates an equality operator
		
		bool is_valid() const
		{
			return true;
		}
	};

	template <graphics::e_pipeline_type _t>
	using pipeline_signature = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
		graphics_pipeline_signature,
		compute_pipeline_signature,
		raytracing_pipeline_signature>>;
#pragma endregion

#pragma region translation
	inline graphics::compute_pipeline_desc translate(const compute_pipeline_signature& signature)
	{
		graphics::compute_pipeline_desc result{};
		return result;
	}

	inline graphics::raytracing_pipeline_desc translate(const raytracing_pipeline_signature& signature)
	{
		graphics::raytracing_pipeline_desc result{};
		return result;
	}

	inline graphics::graphics_pipeline_desc translate(const graphics_pipeline_signature& signature)
	{
		graphics::graphics_pipeline_desc result{};
		result.m_rasterizer.m_cullmode = signature.m_cullmode;
		result.m_rasterizer.m_front_ccw = signature.m_front_ccw;
		result.m_prim_type = signature.m_primitive_type;
		result.m_rasterizer.m_fillmode = signature.m_fillmode;
		result.m_rasterizer.m_forced_samplecount = signature.m_forced_samplecount;
		result.m_sample_mask = signature.m_sample_mask;
		result.m_sample_count = signature.m_sample_count;
		result.m_rasterizer.m_depth_clip_enable = signature.m_depthclip;
		result.m_rasterizer.m_multisample = signature.m_multisample;
		result.m_rasterizer.m_antialiased_line = signature.m_antialiased_line;
		result.m_rasterizer.m_conservative = signature.m_conservative_raster;
		result.m_rasterizer.m_depth_bias = signature.m_depthbias;
		result.m_rasterizer.m_depth_bias_clamp = signature.m_depthbias_clamp;
		result.m_rasterizer.m_slope_depth_bias = signature.m_slope_depthbias;
		result.m_depth_stencil.m_depth_enable = signature.m_depth_enable;
		result.m_depth_stencil.m_stencil_enable = signature.m_stencil_enable;
		result.m_depth_stencil.m_depth_func = signature.m_depth_comparison;
		result.m_format_dsv = signature.m_depth_format;

		for (uint8 i = 0u; i < graphics::k_max_render_targets; ++i)
		{
			result.m_blends[i].m_enabled = signature.m_blend_actives[i];
			result.m_blends[i].m_src = signature.m_blend_sources[i];
			result.m_blends[i].m_dest = signature.m_blend_dests[i];
			result.m_blends[i].m_op = signature.m_blend_ops[i];
			result.m_blends[i].m_srcalpha = signature.m_alpha_sources[i];
			result.m_blends[i].m_destalpha = signature.m_alpha_dests[i];
			result.m_blends[i].m_op_alpha = signature.m_alpha_ops[i];
			result.m_blends[i].m_write_mask = signature.m_blend_writemasks[i];
			result.m_rtvs[i].m_enabled = signature.m_rtv_actives[i];
			result.m_rtvs[i].m_format = signature.m_rtv_formats[i];
		}

		return result;
	}
#pragma endregion

	template <graphics::e_pipeline_type _t>
	class pipeline final
	{
	private:
		using shader_slots = graphics::shader_slots<_t>;
		using e_shader_slot = shader_slots::enum_type;
		using signature_type = pipeline_signature<_t>;
		using pipeline_desc_type = graphics::pipeline_desc<_t>;
		using pipeline_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			graphics::graphics_pipeline,
			graphics::compute_pipeline,
			graphics::raytracing_pipeline>>;

		// graphics:: objects
		graphics::rootsignature_desc m_rootsig_desc{};
		graphics::rootsignature* m_rootsig = nullptr;
		pipeline_desc_type m_desc{};
		pipeline_type* m_pipeline = nullptr;

		// info
		umap<string, uint32>			m_name_to_register;
		umap<string, uint32>			m_name_to_param_idx;
		debug_name						m_name;

		// signature & desc
		signature_type m_signature{};
		
		// shaders
		constexpr static uint8 k_num_shaders = graphics::shader_slots<_t>::count;
		shader_data const*			m_shaders[k_num_shaders]{};
		shader::reflection const*	m_shader_reflections[k_num_shaders]{};
		time::point					m_shader_loadpoints[k_num_shaders]{};
		bool m_needs_rebuild = false;

	public:
		pipeline() = default;
		pipeline(graphics::device& device, const signature_type& signature)
			: m_signature{ signature }
		{
			// setup loadpoints
			time::point begin = time::get_now();
			for (uint8 i = 0u; i < k_num_shaders; ++i)
			{
				m_shader_loadpoints[i] = begin;
			}

			update_shaders(signature);
			rebuild_rootsignature(device, signature);
			rebuild_pipeline(device, signature);
		}

		void update_shaders(graphics::device& device)
		{
			update_shaders(m_signature);

			if (m_needs_rebuild)
			{
				rebuild_rootsignature(device, m_signature);
				rebuild_pipeline(device, m_signature);
			}
		}

		void update_shader(const shader::shader_signature& shader_sig, shader::e_shader_type type)
		{
			static renderer_backend& backend = renderer_backend::get_instance();
			static shader_manager& shaderman = backend.get_shader_manager();

			const e_shader_slot slot = type_to_slot<_t>(type);
			const uint8 slot_index = static_cast<uint8>(slot);

			// setup entrypoint & type
			string shader_id = m_signature.m_shader_identifiers[static_cast<uint8>(slot)];
			if (!shader_id.empty())
			{
				shader::shader_signature shader_signature_copy = shader_sig;
				auto split = str::split(shader_id, "::");
				shader_signature_copy.m_entrypoint = split[1];
				shader_signature_copy.m_filename = split[0];
				shader_signature_copy.m_type = type;
				shader_signature_copy.cache_id();

				// get the appropriate shadermap and store into slot
				shader_map& shadermap = shaderman.get_shadermap(type, shader_signature_copy.m_target);
				const shader_data* new_shader = shadermap.get_shader(shader_signature_copy);

				// if new data is newer than previous loadpoint, flag rebuild
				if (new_shader->m_time_loaded > m_shader_loadpoints[slot_index])
				{
					m_needs_rebuild = true;
				}

				// re-store the pointer & update the loadpoint
				m_shaders[slot_index] = new_shader;
				m_shader_loadpoints[slot_index] = time::get_now();
			}
		}

		// call this to re-fetch shaders from shader_manager and possibly rebuild the pipeline
		void update_shaders(const signature_type& signature)
		{
			shader::shader_signature shader_sig = {};
			shader_sig.m_target = shader::e_shader_target::_6_6;

			if constexpr (_t == graphics::e_pipeline_type::graphics)
			{
				update_shader(shader_sig, shader::e_shader_type::vs);
				update_shader(shader_sig, shader::e_shader_type::ps);
				update_shader(shader_sig, shader::e_shader_type::gs);
				update_shader(shader_sig, shader::e_shader_type::ds);
				update_shader(shader_sig, shader::e_shader_type::hs);
			}
			else if constexpr (_t == graphics::e_pipeline_type::compute)
			{
				update_shader(shader_sig, shader::e_shader_type::cs);
			}
			else if constexpr (_t == graphics::e_pipeline_type::raytracing)
			{
				update_shader(shader_sig, shader::e_shader_type::rgs);
				update_shader(shader_sig, shader::e_shader_type::mss);
				update_shader(shader_sig, shader::e_shader_type::chs);
				update_shader(shader_sig, shader::e_shader_type::ahs);
				update_shader(shader_sig, shader::e_shader_type::ins);
			}

			// collect reflections
			for (uint8 i = 0u; i < k_num_shaders; ++i)
			{
				// assert the non-optional stages
				const bool is_slot_optional = shader_slots::is_optional(static_cast<e_shader_slot>(i));
				influx_assert(m_shaders[i] != nullptr || is_slot_optional);

				if (m_shaders[i] != nullptr)
				{
					m_shader_reflections[i] = &m_shaders[i]->m_reflection;
				}
			}
		}

		// pipeline interfact:
		void set_state(graphics::commandlist& commandlist)
		{
			commandlist.set(m_rootsig, _t);
			commandlist.set(m_pipeline);
		}

		template <typename _constants>
		void set_constants(graphics::commandlist& cmdlist, const string& name, _constants& constants)
		{
			set_constants(cmdlist, name, sizeof(_constants) / sizeof(uint32), &constants);
		}

		void set_constants(graphics::commandlist& cmdlist, const string& name, uint32 num_dwords, void* data)
		{
			cmdlist.set_constants(get_param_index(name), num_dwords, data, _t);
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

	private:
		void rebuild_rootsignature(graphics::device& device, const signature_type& signature)
		{
			// build the root signature:
			graphics::rootsignature_desc rootsig_desc{};
			rootsig_desc.m_direct_indexing = signature.m_bindless;

			auto reflect_resource = [&rootsig_desc, this]
			(const shader::reflection::resource& resource, graphics::e_shader_visibility shader_vis)
			{
				if (!resource.m_name.empty())
					m_name_to_register[resource.m_name] = resource.m_shader_register;

				switch (resource.m_type)
				{
				case shader::reflection::resource::e_type::cbv:
					rootsig_desc.add_root_constants((uint32)resource.m_bytesize / sizeof(uint32),
						resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_constants(resource.m_name);
					break;

				case shader::reflection::resource::e_type::structured:
					rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
						resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_resource_table(resource.m_name);
					break;

				case shader::reflection::resource::e_type::texture:
					rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
						resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_resource_table(resource.m_name);
					break;

				case shader::reflection::resource::e_type::sampler:
					rootsig_desc.add_root_sampler(resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_sampler(resource.m_name);
					break;
				}
			};

			if constexpr (_t == graphics::e_pipeline_type::graphics)
			{
				graphics::e_shader_visibility shader_visibilities[k_num_shaders]
				{
					graphics::e_shader_visibility::vertex,
					graphics::e_shader_visibility::pixel,
					graphics::e_shader_visibility::domain,
					graphics::e_shader_visibility::geometry,
					graphics::e_shader_visibility::hull
				};

				// reflect resources of each shader
				for (uint8 i = 0u; i < k_num_shaders; ++i)
				{
					if (m_shader_reflections[i] != nullptr)
					{
						for (const shader::reflection::resource& resource : m_shader_reflections[i]->m_bound_resources)
						{
							reflect_resource(resource, shader_visibilities[i]);
						}
					}
				}
			}
			else if constexpr (_t == graphics::e_pipeline_type::compute)
			{
				graphics::e_shader_visibility shader_visibilities[k_num_shaders]
				{
					graphics::e_shader_visibility::compute
				};

				// reflect resources of each shader
				for (uint8 i = 0u; i < k_num_shaders; ++i)
				{
					if (m_shader_reflections[i] != nullptr)
					{
						for (const shader::reflection::resource& resource : m_shader_reflections[i]->m_bound_resources)
						{
							reflect_resource(resource, shader_visibilities[i]);
						}
					}
				}
			}
			else if constexpr (_t == graphics::e_pipeline_type::raytracing)
			{
				influx_assert(false);
			}

			// create root signature (destroy previous
			if (m_rootsig) device.release(m_rootsig);

			m_rootsig = device.create_rootsignature(rootsig_desc);
			m_name_to_param_idx = m_rootsig->get_param_idx_table();
			influx_assert(m_rootsig->is_valid());

			m_rootsig_desc = rootsig_desc;
		}

		void rebuild_pipeline(graphics::device& device, const signature_type& signature)
		{
			pipeline_desc_type pipeline_desc{};
			pipeline_desc = translate(signature);

			// set shaders
			for (uint8 i = 0u; i < k_num_shaders; ++i)
			{
				if (m_shaders[i] != nullptr)
				{
					using shader_slot_enum = graphics::shader_slots<_t>::enum_type;
					const shader_slot_enum shader_slot = static_cast<shader_slot_enum>(i);
					pipeline_desc.m_shaders.set(shader_slot, m_shaders[i]->m_bytecode);
				}
			}

			// destroy previous
			if (m_pipeline) device.release(m_pipeline);

			// set vs input layout
			if constexpr (_t == graphics::e_pipeline_type::graphics)
			{
				// parse the input elements from vertex shader reflection:
				constexpr uint8 vertex_shader_idx = static_cast<uint8>(graphics::e_graphics_shader_slots::vs);
				const shader_data& vertex_shader = *m_shaders[vertex_shader_idx];
				const shader::reflection& vertex_reflection = *m_shader_reflections[vertex_shader_idx];
				for (uint32 i = 0u; i < vertex_reflection.m_input_params.size(); ++i)
				{
					const shader::reflection::input_param& param = vertex_reflection.m_input_params[i];

					// derive the format
					graphics::e_format format;
					switch (param.m_num_floats)
					{
					case 1u: format = graphics::e_format::r32; break;
					case 2u: format = graphics::e_format::rg32; break;
					case 3u: format = graphics::e_format::rgb32; break;
					case 4u: format = graphics::e_format::rgba32; break;
					default:
						influx_assert(false); // WOAH!
						break;
					}

					pipeline_desc.add_input_element(
						param.m_semantic_name,
						param.m_semantic_index,
						format,
						0u,
						false,
						0u);
				}

				m_pipeline = device.create_graphics_pipeline(m_rootsig, pipeline_desc);
			}
			else if constexpr (_t == graphics::e_pipeline_type::compute)
			{
				m_pipeline = device.create_compute_pipeline(m_rootsig, pipeline_desc);
			}
			else if constexpr (_t == graphics::e_pipeline_type::raytracing)
			{
				static_assert("todo: raytracing pipeline");
			}

			m_desc = pipeline_desc;
		}

		shader_data const* get_shader(graphics::shader_slots<_t>::enum_type slot) const
		{
			return m_shaders[static_cast<uint8>(slot)];
		}
	};
	
	using graphics_pipeline		= pipeline<graphics::e_pipeline_type::graphics>;
	using compute_pipeline		= pipeline<graphics::e_pipeline_type::compute>;
	using raytracing_pipeline	= pipeline<graphics::e_pipeline_type::raytracing>;
}

// Specialize std::hash for shader_signature
namespace std {
	template <>
	struct hash<influx::renderer::graphics_pipeline_signature> {
		std::size_t operator()(const influx::renderer::graphics_pipeline_signature& sig) const 
		{
			return sig.get_hash();
		}
	};

	template <>
	struct hash<influx::renderer::compute_pipeline_signature> {
		std::size_t operator()(const influx::renderer::compute_pipeline_signature& sig) const
		{
			return sig.get_hash();
		}
	};

	template <>
	struct hash<influx::renderer::raytracing_pipeline_signature> {
		std::size_t operator()(const influx::renderer::raytracing_pipeline_signature& sig) const
		{
			return sig.get_hash();
		}
	};
}