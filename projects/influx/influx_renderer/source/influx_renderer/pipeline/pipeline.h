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
#pragma region
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
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::compute)
		{
			switch (type)
			{
			case shader::e_shader_type::cs: return graphics::e_compute_shader_slots::cs;
			}
		}
		else if constexpr (_t == graphics::e_pipeline_type::raytracing)
		{
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
	}
#pragma endregion
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
		string m_shader_identifiers[ graphics::graphics_shaderslots::num ]{};

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
		uint32 m_rtv_formats[k_max_num_rendertargets]	= { format::rgba8, 0u, 0u, 0u, 0u, 0u, 0u, 0u };

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
		string m_shader_identifiers[graphics::compute_shaderslots::num]{};
		bool operator==(const compute_pipeline_signature&) const = default; // Automatically generates an equality operator
	};

	struct raytracing_pipeline_signature final
	{
		bool m_bindless = false;
		string m_shader_identifiers[graphics::raytracing_shaderslots::num]{};
		bool operator==(const raytracing_pipeline_signature&) const = default; // Automatically generates an equality operator
	};

	template <graphics::e_pipeline_type _t>
	using pipeline_signature = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
		graphics_pipeline_signature,
		compute_pipeline_signature,
		raytracing_pipeline_signature>>;

	constexpr static graphics::e_cull_mode translate(graphics_pipeline_signature::cullmode mode)
	{
		switch (mode)
		{
		case graphics_pipeline_signature::cullmode::back: return graphics::e_cull_mode::back;
		case graphics_pipeline_signature::cullmode::front: return graphics::e_cull_mode::front;
		case graphics_pipeline_signature::cullmode::none: return graphics::e_cull_mode::nocull;
		}
		return graphics::e_cull_mode::count;
	}

	constexpr static graphics::e_format translate(graphics_pipeline_signature::format format)
	{
		switch (format)
		{
		case graphics_pipeline_signature::format::rgba8: return graphics::e_format::rgba8;
		case graphics_pipeline_signature::format::r32: return graphics::e_format::r32;
		case graphics_pipeline_signature::format::rg32: return graphics::e_format::rg32;
		case graphics_pipeline_signature::format::rgb32: return graphics::e_format::rgb32;
		case graphics_pipeline_signature::format::rgba32: return graphics::e_format::rgba32;
		case graphics_pipeline_signature::format::d32: return graphics::e_format::d32;
		case graphics_pipeline_signature::format::u16: return graphics::e_format::u16;
		case graphics_pipeline_signature::format::u32: return graphics::e_format::u32;
		case graphics_pipeline_signature::format::u32_4: return graphics::e_format::rgba_u32;
		}
		return {};
	}

	namespace detail
	{
		class pipeline
		{
		public:
			virtual graphics::e_pipeline_type get_type() const = 0;

		protected:
			graphics::rootsignature_desc m_rootsig_desc{};
			graphics::rootsignature* m_rootsig = nullptr;
			umap<string, uint32> m_name_to_register;
			umap<string, uint32> m_name_to_param_idx;
			debug_name m_name;
		};

		template <graphics::e_pipeline_type _t>
		class tpipeline : public pipeline
		{
		public:
			using signature_type = pipeline_signature<_t>;
			using pipeline_desc_type = graphics::pipeline_desc<_t>;
			using pipeline_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
				graphics::graphics_pipeline,
				graphics::compute_pipeline,
				graphics::raytracing_pipeline>>;

			signature_type m_signature = nullptr;
			pipeline_type* m_pipeline = nullptr;
			pipeline_desc_type m_desc{};

			constexpr static uint8 k_num_shaders = graphics::shader_slots<_t>::count;
			shader_data const* m_shaders[k_num_shaders]{};
			shader::reflection const* m_shader_reflections[k_num_shaders]{};

			tpipeline(graphics::device& device, const signature_type& signature)
				: m_signature{signature}
			{
				gather_shaders(signature);
				build_rootsignature(device, signature);
				build_pipeline(device, signature);
			}

			void gather_shaders(const signature_type& signature)
			{
				renderer_backend& backend = renderer_backend::get_instance();
				static shader_manager& shaderman = backend.get_shader_manager();

				shader::shader_signature shader_sig = {};
				shader_sig.m_target = shader::e_shader_target::_6_6;

				auto get_and_store_shader = [this](shader::shader_signature signature, shader::e_shader_type type)
				{
					const graphics::shader_slots<_t>::enum_type slot = type_to_slot<_t>(type);

					// setup entrypoint & type
					string shader_id = m_signature.m_shader_identifiers[static_cast<uint8>(slot)];
					if (!shader_id.empty())
					{
						auto split = str::split(shader_id, "::");
						signature.m_entrypoint = split[1];
						signature.m_filename = split[0];
						signature.m_type = type;
						signature.update_id();

						// get the appropriate shadermap and store into slot
						shader_map& shadermap = shaderman.get_shadermap(type, signature.m_target);
						store_shaderdata(slot, shadermap.get_shader(signature));
					}
				};

				if constexpr (_t == graphics::e_pipeline_type::graphics)
				{
					get_and_store_shader(shader_sig, shader::e_shader_type::vs);
					get_and_store_shader(shader_sig, shader::e_shader_type::ps);
					influx_assert(get_shader(graphics::e_graphics_shader_slots::vs) != nullptr);
					influx_assert(get_shader(graphics::e_graphics_shader_slots::ps) != nullptr);

					get_and_store_shader(shader_sig, shader::e_shader_type::gs);
					get_and_store_shader(shader_sig, shader::e_shader_type::ds);
					get_and_store_shader(shader_sig, shader::e_shader_type::hs);
				}
				else if constexpr (_t == graphics::e_pipeline_type::compute)
				{
					get_and_store_shader(shader_sig, shader::e_shader_type::cs);
				}
				else if constexpr (_t == graphics::e_pipeline_type::raytracing)
				{
					// ...
				}

				// gather reflections
				for (uint8 i = 0u; i < k_num_shaders; ++i)
				{
					if (m_shaders[i] != nullptr)
					{
						m_shader_reflections[i] = &m_shaders[i]->m_reflection;
					}
				}
			}

			template <graphics::shader_slots<_t>::enum_type _s>
			bool is_shader_dirty()
			{
				static constexpr uint32 index = static_cast<uint32>(_s);
				if (m_shaders[index] == nullptr)
				{
					return true;
				}

				return false;
			}

			void build_rootsignature(graphics::device& device, const signature_type& signature)
			{
				// build the root signature:
				graphics::rootsignature_desc& rootsig_desc = m_rootsig_desc;
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
				
				// create root signature
				m_rootsig = device.create_rootsignature(rootsig_desc);
				m_name_to_param_idx = m_rootsig->get_param_idx_table();
				influx_assert(m_rootsig->is_valid());
			}

			void build_pipeline(graphics::device& device, const signature_type& signature)
			{
				graphics::pipeline_desc<_t> pipeline_desc{};

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

				if constexpr (_t == graphics::e_pipeline_type::graphics)
				{
					pipeline_desc.m_rasterizer.m_cullmode = translate((graphics_pipeline_signature::cullmode)signature.m_cullmode);
					pipeline_desc.m_rasterizer.m_front_ccw = signature.m_front_ccw;
					pipeline_desc.m_prim_type = (graphics::e_primitive_topology_type)signature.m_primitive_type;
					pipeline_desc.m_rasterizer.m_fillmode = (graphics::e_fill_mode)signature.m_fillmode;
					pipeline_desc.m_rasterizer.m_forced_samplecount = signature.m_forced_samplecount;
					pipeline_desc.m_sample_mask = signature.m_sample_mask;
					pipeline_desc.m_sample_count = signature.m_sample_count;
					pipeline_desc.m_rasterizer.m_depth_clip_enable = signature.m_depthclip;
					pipeline_desc.m_rasterizer.m_multisample = signature.m_multisample;
					pipeline_desc.m_rasterizer.m_antialiased_line = signature.m_antialiased_line;
					pipeline_desc.m_rasterizer.m_conservative = signature.m_conservative_raster;
					pipeline_desc.m_rasterizer.m_depth_bias = signature.m_depthbias;
					pipeline_desc.m_rasterizer.m_depth_bias_clamp = signature.m_depthbias_clamp;
					pipeline_desc.m_rasterizer.m_slope_depth_bias = signature.m_slope_depthbias;
					pipeline_desc.m_depth_stencil.m_depth_enable = signature.m_depth_enable;
					pipeline_desc.m_depth_stencil.m_stencil_enable = signature.m_stencil_enable;
					pipeline_desc.m_depth_stencil.m_depth_func = (graphics::e_comparison_func)signature.m_depth_comparison;
					pipeline_desc.m_format_dsv = translate((graphics_pipeline_signature::format)signature.m_depth_format);
					for (uint8 i = 0u; i < 8u; ++i)
					{
						pipeline_desc.m_blends[i].m_enabled = signature.m_blend_actives[i];
						pipeline_desc.m_blends[i].m_src = (graphics::e_blend)signature.m_blend_sources[i];
						pipeline_desc.m_blends[i].m_dest = (graphics::e_blend)signature.m_blend_dests[i];
						pipeline_desc.m_blends[i].m_op = (graphics::e_blendop)signature.m_blend_ops[i];
						pipeline_desc.m_blends[i].m_srcalpha = (graphics::e_blend)signature.m_alpha_sources[i];
						pipeline_desc.m_blends[i].m_destalpha = (graphics::e_blend)signature.m_alpha_dests[i];
						pipeline_desc.m_blends[i].m_op_alpha = (graphics::e_blendop)signature.m_alpha_ops[i];
						pipeline_desc.m_blends[i].m_write_mask = signature.m_blend_writemasks[i];
						pipeline_desc.m_rtvs[i].m_enabled = signature.m_rtv_actives[i];
						pipeline_desc.m_rtvs[i].m_format = translate((graphics_pipeline_signature::format)signature.m_rtv_formats[i]);
					}

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
					
				}
			}

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

			virtual graphics::e_pipeline_type get_type() const override { return _t; }

			void store_shaderdata(graphics::shader_slots<_t>::enum_type slot, shader_data const* data)
			{
				m_shaders[static_cast<uint8>(slot)] = data;
			}

			shader_data const* get_shader(graphics::shader_slots<_t>::enum_type slot) const
			{
				return m_shaders[static_cast<uint8>(slot)];
			}
		};
	}
	
	using graphics_pipeline = detail::tpipeline<graphics::e_pipeline_type::graphics>;
	using compute_pipeline = detail::tpipeline<graphics::e_pipeline_type::compute>;
	using raytracing_pipeline = detail::tpipeline<graphics::e_pipeline_type::raytracing>;
}