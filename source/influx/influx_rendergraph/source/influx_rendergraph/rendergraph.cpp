#include "rendergraph_pch.h"

// influx::core
#include "core/scope.h"
#include "core/enum.h" // has_any_flag()...
#include "core/log.h"

// influx::rendergraph
#include "rendergraph.h"
#include "rgpass.h"
#include "rgpool.h"
#include "rgresources.h"
#include "rgpass_builder.h"

// stl
#include <stack>

namespace influx::rendergraph
{
#pragma region translation
	constexpr rhi_load_op translate(const e_rg_load load)
	{
		switch (load)
		{
		case e_rg_load::clear: return rhi_load_op::clear;
		case e_rg_load::discard: return rhi_load_op::discard;
		case e_rg_load::preserve: return rhi_load_op::preserve;
		case e_rg_load::no_access: return rhi_load_op::no_access;
		}
		return rhi_load_op::count;
	}

	constexpr rhi_store_op translate(const e_rg_store store)
	{
		switch (store)
		{
		case e_rg_store::resolve: return rhi_store_op::resolve;
		case e_rg_store::discard: return rhi_store_op::discard;
		case e_rg_store::preserve: return rhi_store_op::preserve;
		case e_rg_store::no_access: return rhi_store_op::no_access;
		}
		return rhi_store_op::count;
	}

	texture_desc rendergraph::translate_texture_desc(const rhi_resource& resource)
	{
		texture_desc desc{};
		desc.m_array_size = resource.get_arraysize();
		desc.m_bindflags = {};
		desc.m_depth = resource.get_depth();
		desc.m_width = resource.get_width();
		desc.m_heigth = resource.get_height();
		desc.m_init_state = resource.get_resource_state();
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_allow_uav = resource.allows_uav();
		return desc;
	}
	buffer_desc rendergraph::translate_buffer_desc(const rhi_resource& resource)
	{
		buffer_desc desc{};
		desc.m_bytesize = resource.get_bytesize();
		desc.m_bytestride = resource.get_bytestride();
		return desc;
	}
#pragma endregion

	rendergraph::rendergraph(const global_config& config, rhi_device& device)
	{
		m_config = config;
		m_pool = new rgpool(device, config);
	}

	void rendergraph::cleanup(rhi_device& device)
	{
#if 0
		// release the device objects
		for (auto& pair : m_texid_to_deviceobjects_map)
		{
			for (uint8 i = 0u; i < k_num_descriptor_types; ++i)
			{
				if (pair.second[i] != nullptr)
				{
					// device.release(pair.second[i]);
					pair.second[i] = nullptr;
				}
			}
		}
		for (auto& pair : m_bufid_to_deviceobjects_map)
		{
			for (uint8 i = 0u; i < k_num_descriptor_types; ++i)
			{
				if (pair.second[i] != nullptr)
				{
					// device.release(pair.second[i]);
					pair.second[i] = nullptr;
				}
			}
		}
#endif
		// cleanup pool descriptor heaps & such
		m_pool->cleanup(device);
	}

	rendergraph::~rendergraph()
	{
		for (rgtexture*& texture : m_textures)
		{
			delete texture;
			texture = nullptr;
		}

		for (rgbuffer*& buffer : m_buffers)
		{
			delete buffer;
			buffer = nullptr;
		}

		delete m_pool;
		m_pool = nullptr;
	}
	
	void rendergraph::build()
	{
		// build an adjacency list
		build_adjacency();
		
		// sort the passes topologically
		sort_topological();

		// cull stubbies
		cull_passes();

		// setup the layers of passes
		build_layers();

		calc_resource_lifetimes();

		// setup child creates & destroys
		for (rglayer& layer : m_layers)
		{
			for (const rgpass& pass : layer.m_passes)
			{
				// if (pass.is_culled()) continue;

				layer.m_texture_creates.insert(pass.m_texture_creates.begin(), pass.m_texture_creates.end());
				layer.m_texture_destroys.insert(pass.m_texture_destroys.begin(), pass.m_texture_destroys.end());
				for (auto [resource, state] : pass.m_texture_state_map)
				{
					layer.m_texture_to_state_map[resource] |= state;
				}

				layer.m_buffer_creates.insert(pass.m_buffer_creates.begin(), pass.m_buffer_creates.end());
				layer.m_buffer_destroys.insert(pass.m_buffer_destroys.begin(), pass.m_buffer_destroys.end());
				for (auto [resource, state] : pass.m_buffer_state_map)
				{
					layer.m_buffer_to_state_map[resource] |= state;
				}
			}
		}
	}

	result<> rendergraph::execute(rhi_commandlist& commandlist, rhi_device& device)
	{
		using result_type = result<>;

		m_pool->tick();

		// start recording commandlist if not done already
		if (commandlist.is_recording() == false)
		{
			auto start_commandlist = commandlist.start(&device);
			if (!start_commandlist)
				return result<>::make_error("failed starting the commandlist!");
		}

		// validation checks
		const bool is_valid = execute_validation_checks();
		if (!is_valid) 
			return result<>::make_error("rendergraph failed validation checks!");

		// execute layers
		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			const rglayer& layer = m_layers[layer_idx];

			// create declared resources & create descriptors/views
			for (const rgtexture_id& tex_id : layer.m_texture_creates)
			{
				rgtexture* texture = get_texture(tex_id);
				texture->m_resource = m_pool->allocate_texture_resource(device, texture->m_desc).get();
				texture->m_resource->set_name(texture->m_name);
			}
			for (const rgbuffer_id& buff_id : layer.m_buffer_creates)
			{
				rgbuffer* buffer = get_buffer(buff_id);
				buffer->m_resource = m_pool->allocate_buffer_resource(device, buffer->m_desc).get();
				buffer->m_resource->set_name(buffer->m_name);
			}

			// create imported resources descriptors/views
			for (uint64 i = 0; i < m_textures.size(); ++i)
			{ 
				create_texture_views(device, m_textures[i]->m_id).get();
			}
			for (uint64 i = 0; i < m_buffers.size(); ++i)
			{
				create_buffer_views(device, m_buffers[i]->m_id).get();
			}

			// transition resources to appropriate state
			for (auto const& [tex_id, state] : layer.m_texture_to_state_map)
			{
				rgtexture* texture = get_texture(tex_id);
				rhi_resource* resource = texture->m_resource;
				if (resource->get_resource_state() != state)
				{
					resource->transition(commandlist, state);
				}
			}
			for (auto const& [buff_id, state] : layer.m_buffer_to_state_map)
			{
				rgbuffer* buffer = get_buffer(buff_id);
				rhi_resource* resource = buffer->m_resource;
				if (resource->get_resource_state() != state)
				{
					resource->transition(commandlist, state);
				}
			}

#if INFLUX_RG_BACKEND_GRAPHICS
			commandlist.flush_barriers();
#endif

			// execute passes
#if INFLUX_RG_BACKEND_GRAPHICS
			for (size_t pass_idx = 0u; pass_idx < layer.m_passes.size(); ++pass_idx)
			{
				const rgpass& pass = layer.m_passes[pass_idx];
				if (pass.is_culled())
				{
					// todo... (for now we're not culling passes)
				}

				rgpass_context context{ *this, commandlist, pass };
				if (pass.get_type() == e_rgpass_type::graphics)
				{
					// construct renderpass arguments
					graphics::renderpass_args args{};
					args.m_width = pass.get_width();
					args.m_height = pass.get_height();
					args.m_legacy = false;

					bool is_pass_dimensions_valid = args.m_width <= 0u || args.m_height <= 0u;

					// gather colour target attachment infos
					math::uint2 first_rt_dimensions = {};
					args.m_color_attachments.reserve(pass.m_rtvs.size());
					for (const auto& rtv : pass.m_rtvs)
					{
						const rgtexture* texture = m_textures[rtv.m_texture_id.m_id];
						influx::graphics::color_attachment attachment{};
						attachment.m_load = translate(rtv.m_access.m_load);
						attachment.m_store = translate(rtv.m_access.m_store);

						const rhi_descriptor rtv_descriptor = get_rtv(rtv.m_texture_id);
						if (rtv_descriptor == 0u) 
							return result_type::make_error("rendergraph::execute() >> invalid RTV!");
						
						attachment.m_rtv_descriptor = rtv_descriptor;
						// load:preserve info
						if (rtv.m_access.m_load == e_rg_load::preserve) {} // nothing to declare

						// load:clear info
						if (rtv.m_access.m_load == e_rg_load::clear)
						{
							memcpy(attachment.m_clear.m_data, rtv.m_access.m_load_clear.m_colour.m_data, sizeof(FLOAT[4]));
						}

						// store:resolve info
						if (rtv.m_access.m_store == e_rg_store::resolve)
						{
							rgtexture* resolve_source_texture = get_texture(rtv.m_access.m_store_resolve.m_source_texture);
							rgtexture* resolve_dest_texture = get_texture(rtv.m_access.m_store_resolve.m_dest_texture);

							auto& resolve = attachment.m_resolve;
							resolve.m_format = rtv.m_access.m_store_resolve.m_dest_format;
							resolve.m_source = resolve_source_texture->m_resource;
							resolve.m_dest = resolve_dest_texture->m_resource;
							resolve.m_keep_source = rtv.m_access.m_store_resolve.m_keep_source;
						}

						// store:preserve
						if (rtv.m_access.m_store == e_rg_store::preserve) {} // nothing to declare
						args.m_color_attachments.push_back(attachment);
					}

					// gather single depth attachment info
					auto& depth_attachment = args.m_depth_attachment;
					depth_attachment.m_is_enabled = pass.m_dsv.m_is_enabled;
					if (depth_attachment.m_is_enabled)
					{
						const auto& dsv = pass.m_dsv;
						depth_attachment.m_dsv_descriptor = get_dsv(dsv.m_texture_id);
						depth_attachment.m_depth_load = translate(dsv.m_depth_access.m_load);
						depth_attachment.m_depth_store = translate(dsv.m_depth_access.m_store);
						depth_attachment.m_stencil_load = translate(dsv.m_stencil_access.m_load);
						depth_attachment.m_stencil_store = translate(dsv.m_stencil_access.m_store);
						depth_attachment.m_depth_clear = dsv.m_depth_clear;
						depth_attachment.m_stencil_clear = dsv.m_stencil_clear;

						// load:clear info
						if (dsv.m_depth_access.m_load == e_rg_load::clear)
						{
							depth_attachment.m_depth_clear = dsv.m_depth_access.m_load_clear.m_depth;
						}
					}

					// dispatch the renderpass
					influx_scope("renderpass");
					commandlist.renderpass_begin(args);

					// implicit viewport / rect
					if (args.m_allow_implicit_viewport_set)
					{
						graphics::viewport viewport{};
						viewport.m_width = (float)args.m_width;
						viewport.m_height = (float)args.m_height;
						viewport.m_depth_max = 1.0f;
						viewport.m_depth_min = 0.0f;

						viewport.m_width = viewport.m_width == 0u ? pass.m_rtvs[0].m_dimensions.x : viewport.m_width;
						viewport.m_height = viewport.m_height == 0u ? pass.m_rtvs[0].m_dimensions.y : viewport.m_height;

						commandlist.set_viewport(viewport);
					}
					if (args.m_allow_implicit_viewrect_set)
					{
						graphics::rect rect{};
						rect.m_right = args.m_width;
						rect.m_bottom = args.m_height;
						rect.m_top = 0u;
						rect.m_left = 0u;

						rect.m_right = rect.m_right == 0u ? pass.m_rtvs[0].m_dimensions.x : rect.m_right;
						rect.m_bottom = rect.m_bottom == 0u ? pass.m_rtvs[0].m_dimensions.y : rect.m_bottom;

						commandlist.set_scissor_rect(rect);
					}

					pass.execute(context);
					commandlist.renderpass_end();
				}
				else
				{
					// compute / raytracing passes have nothing fancy going on 
					// in terms of renderpasses, just call their execute
					pass.execute(context);
				}
			}
#endif
			// execute destroys
			for (const rgtexture_id& tex_id : layer.m_texture_destroys)
			{
				rgtexture* texture = get_texture(tex_id);
				if (!texture->is_imported()) m_pool->release_texture(device, *texture->m_resource);
			}
			for (const rgbuffer_id& buff_id : layer.m_buffer_destroys)
			{
				rgbuffer* buffer = get_buffer(buff_id);
				if (!buffer->is_imported()) m_pool->release_buffer(device, *buffer->m_resource);
			}
		}

		return {};
	}

	rgpass* rendergraph::add_pass( e_rgpass_type type, const rgpass_builder_clb& builder_clb, const rgpass_process_clb& process_clb)
	{
		m_passes.emplace_back(rgpass(builder_clb, process_clb, type));
		rgpass& new_pass = m_passes.back();

		rgpass_id new_id = m_passes.size() - 1u;
		new_pass.set_id(new_id);

		m_id_to_pass_map[new_id] = &new_pass;

		// build pass
		rgpass_builder new_builder{ *this, new_pass };
		new_pass.build(new_builder);

		return &new_pass;
	}

	rgpass* rendergraph::add_copypass(rhi_resource* source, rhi_resource* dest, bool keep_source)
	{
		import_texture(dest);
		import_texture(source);

		static rgtex_copysrc_id src_tex_id{};
		static rgtex_copydst_id dst_tex_id{};
		auto* pass = add_pass( e_rgpass_type::compute,
			[&source, &dest, keep_source](rgpass_builder& builder)
			{
				src_tex_id = builder.read_copysrc_texture(source->get_name()).get();
				dst_tex_id = builder.write_copydst_texture(dest->get_name()).get();
				builder.set_viewport(dest->get_width(), dest->get_height());
			},
			[](rgpass_context& context) 
			{
				rhi_resource* src_resource = context.get_copysrc(src_tex_id).get().m_resource;
				rhi_resource* dst_resource = context.get_copydst(dst_tex_id).get().m_resource;
				context.get_commandlist().copy_resource(*src_resource, *dst_resource);
			});

		pass->set_name("copy");
		return pass;
	}

	rgpass* rendergraph::add_clear_pass(rhi_resource* dest, const clear_args& args)
	{
		import_texture(dest);

		auto* pass = add_pass(e_rgpass_type::graphics,
			[dest, &args](rgpass_builder& builder)
			{
				rgaccess access{};
				access.m_load = e_rg_load::clear;
				access.m_store = e_rg_store::preserve;
				access.m_load_clear.m_colour = args.m_colour;
				builder.write_rendertarget(dest->get_name(), access);
				builder.set_viewport(dest->get_width(), dest->get_height());
			},
			[](rgpass_context& context) {});

		pass->set_name("clear");
		return pass;
	}

	result<> rendergraph::import_texture(rhi_resource* resource)
	{
		return import_texture(resource->get_name(), resource);
	}

	result<> rendergraph::import_buffer(rhi_resource* resource)
	{
		return import_buffer(resource->get_name(), resource);
	}

	result<> rendergraph::import_texture(const rgname& name, rhi_resource* resource)
	{
		if (resource == nullptr || resource->is_valid() == false)
			return result<>::make_error("error: importing invalid resource!");

		if (m_texture_name_to_id_map.contains(name) == false)
		{
			rgtexture* new_texture = new rgtexture();
			rgtexture_id new_id = m_textures.size();
			new_texture->m_desc = translate_texture_desc(*resource);
			new_texture->m_id = new_id;
			new_texture->m_resource = resource;
			new_texture->m_is_imported = true;
			new_texture->m_name = name;
			m_textures.emplace_back(new_texture);
			m_texture_name_to_id_map[name] = new_id;
			m_id_to_texture_map[new_id] = m_textures.back();
		}
		else
		{
			// overwrite existing
			rgtexture_id id = m_texture_name_to_id_map[name];
			rgtexture*& texture = m_id_to_texture_map[id];
			texture->m_name = name;
			texture->m_desc = translate_texture_desc(*resource);
			texture->m_resource = resource;
		}

		return {};
	}

	result<> rendergraph::import_buffer(const rgname& name, rhi_resource* resource)
	{
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			rgbuffer* new_buffer = new rgbuffer();
			rgbuffer_id new_id = m_buffers.size();
			new_buffer->m_desc = translate_buffer_desc(*resource);
			new_buffer->m_id = new_id;
			new_buffer->m_is_imported = true;
			new_buffer->m_resource = resource;
			new_buffer->m_name = name;
			m_buffers.emplace_back(new_buffer);
			m_buffer_name_to_id_map[name] = new_id;
			m_id_to_buffer_map[new_id] = m_buffers.back();
		}
		else
		{
			// overwrite existing
			rgbuffer_id id = m_buffer_name_to_id_map[name];
			rgbuffer*& buffer = m_id_to_buffer_map[id];
			buffer->m_desc = translate_buffer_desc(*resource);
			buffer->m_resource = resource;
		}

		return {};
	}

	result<> rendergraph::remove_imported_texture(const rgname& name)
	{
		if (m_texture_name_to_id_map.contains(name) == true)
		{
			const auto& texid = m_texture_name_to_id_map[name];
			if (m_id_to_texture_map[texid]->is_imported())
			{
				m_id_to_texture_map.erase(texid);
				m_texture_name_to_id_map.erase(name);
			}
			else return result<>::make_error("error: found texture is not imported!");
		}
		return result<>::make_warning({}, "warning: texture not found!");
	}

	result<> rendergraph::remove_imported_buffer(const rgname& name)
	{
		if (m_buffer_name_to_id_map.contains(name) == true)
		{
			const auto& bufferid = m_buffer_name_to_id_map[name];
			if (m_id_to_buffer_map[bufferid]->is_imported())
			{
				m_id_to_buffer_map.erase(bufferid);
				m_buffer_name_to_id_map.erase(name);
			}
			else return result<>::make_error("error: found buffer is not imported!");
		}
		return result<>::make_warning({}, "error: buffer not found!");
	}

	void rendergraph::reset_resources()
	{
		m_buffers.clear();
		m_textures.clear();
		m_id_to_texture_map.clear();
		m_id_to_buffer_map.clear();

		m_texture_name_to_id_map.clear();
		m_buffer_name_to_id_map.clear();

		// m_texid_to_deviceobjects_map.clear();
		// m_bufid_to_deviceobjects_map.clear();

		m_texid_to_viewdesc_map.clear();
		m_texid_to_descriptors_map.clear();

		m_bufid_to_viewdesc_map.clear();
		m_bufid_to_descriptors_map.clear();

		// if all our resources are reset, our graph also is required to reset
		reset_graph();
	}

	void rendergraph::reset_graph()
	{
		m_pool->free_all_descriptors();

		// clear pass relation info
		m_passes.clear();
		m_layers.clear();

		for (uint64 i = 0; i < m_textures.size(); ++i)
		{
			m_textures[i]->reset_graph();
		}
		for (uint64 i = 0; i < m_buffers.size(); ++i)
		{
			m_buffers[i]->reset_graph();
		}

		m_adjacency_lists.clear();
		m_id_to_pass_map.clear();
		m_topo_sorted_passes.clear();
		m_buffer_uav_counter_map.clear();
		m_rtid_to_clear_map.clear();

		// clear the descriptors linked to textures & buffers
		for (auto& pair : m_texid_to_viewdesc_map)
		{
			const rgtexture_id& id = pair.first;
			texture_view_desc* viewdescs = pair.second;
			for (uint32 i = 0u; i < k_num_descriptor_types; ++i)
			{
				viewdescs[i].clear();
			}
		}
		for (auto& pair : m_bufid_to_viewdesc_map)
		{
			const rgbuffer_id& id = pair.first;
			buffer_view_desc* viewdescs = pair.second;
			for (uint32 i = 0u; i < k_num_descriptor_types; ++i)
			{
				viewdescs[i].clear();
			}
		}
	}

	string rendergraph::make_dump()
	{
		string result{};

		result += "// ====================	//\n";
		result += "// rendergraph dump		//\n";
		result += "// ====================  //\n";

		for (rglayer& layer : m_layers)
		{
			uint32 counter = 0u;
			for (rgpass& pass : layer.m_passes)
			{
				result;
				result += "\t";

				counter++;
			}

			result += "\n";
		}

		result += "\n\n";
		result += make_resources_dump();

		return result;
	}


	string rendergraph::make_dotfile()
	{	
		std::stringstream stream{};
		stream << "digraph rendergraph {\n";
		stream << "compound=true;\n";
		stream << "ranksep=0.4;\n";

		static const auto get_id = [](const debug_name& name)
			{
				return std::to_string(name.get_hash());
			};

		// all resources
		for (uint32 i = 0u; i < m_passes.size(); ++i)
		{
			const rgpass& pass = m_passes[i];
			stream << get_id(pass.m_name) << " ";
			stream << "[shape=oval, label=\"" << pass.m_name.get_string().c_str() << "\"];";
			stream << "\n";
		}
		for (uint32 i = 0u; i < m_textures.size(); ++i)
		{
			const auto& texture = m_textures[i];
			stream << get_id(texture->m_name) << " ";
			stream << "[shape=box, label=\"" << texture->m_name.get_string().c_str() << "\"];";
			stream << "\n";
		}
		for (uint32 i = 0u; i < m_buffers.size(); ++i)
		{
			const auto& buffer = m_buffers[i];
			stream << get_id(buffer->m_name) << " ";
			stream << "[shape=box, label=\"" << buffer->m_name.get_string().c_str() << "\"];";
			stream << "\n";
		}

		for (uint32 i = 0u; i < m_layers.size(); ++i)
		{
			for (uint32 i = 0u; i < m_passes.size(); ++i)
			{
				const rgpass& pass = m_passes[i];
				for (const auto& read : pass.m_buffer_reads)
				{
					const auto& buffer = get_buffer(read.m_id);
					stream << get_id(buffer->m_name) << " -> "
						<< get_id(pass.m_name) << "\n";
				}
				for (const auto& read : pass.m_texture_reads)
				{
					const auto& texture = get_texture(read.m_id);
					stream << get_id(texture->m_name) << " -> "
						<< get_id(pass.m_name) << "\n";
				}
				for (const auto& write : pass.m_buffer_writes)
				{
					const auto& buffer = get_buffer(write.m_id);
					stream << get_id(pass.m_name) << " -> "
						<< get_id(buffer->m_name) << "\n";
				}
				for (const auto& write : pass.m_texture_writes)
				{
					const auto& texture = get_texture(write.m_id);
					stream << get_id(pass.m_name) << " -> "
						<< get_id(texture->m_name) << "\n";
				}
			}
		}

		stream << "}";
		return stream.str();
	}

	string rendergraph::make_resources_dump()
	{
		string result{};

		result += "// ====================	//\n";
		result += "// rendergraph resources //\n";
		result += "// ====================  //\n";

		result += "// textures: \n";
		for (const auto& texture : m_textures)
		{
			result += "- " + to_string(texture->m_id.m_id);
			result += "\n";
		}

		result += "// buffers: \n";
		for (const auto& buffer : m_buffers)
		{
			result += "- " + to_string(buffer->m_id.m_id);
			result += "\n";
		}

		return result;
	}

	vector<rgtexture_info> rendergraph::get_textures() const
	{
		vector<rgtexture_info> result{};
		for (const auto& texture : m_textures)
		{
			result.push_back({
				.m_name = (const char*)texture->m_name
				});
		}
		return result;
	}

	vector<rgbuffer_info> rendergraph::get_buffers() const
	{
		vector<rgbuffer_info> result{};
		for (const auto& buffer : m_buffers)
		{
			result.push_back({
				.m_name = (const char*)buffer->m_name
				});
		}
		return result;
	}

	result<> rendergraph::bind_ext_descheap(e_ext_descheap_slot slot, rhi_descheap& heap, bool allow_override)
	{
		return m_pool->bind_ext_descheap(slot, heap, allow_override);
	}
	result<> rendergraph::unbind_ext_descheap(e_ext_descheap_slot slot)
	{
		return m_pool->unbind_ext_descheap(slot);
	}
	bool rendergraph::is_ext_descheap_bound(e_ext_descheap_slot slot) const
	{
		return m_pool->is_ext_descheap_bound(slot);
	}

	/* creates views (rtv/dsv/srv/samp) based on how the resource will be used in our rendergraph */
	result<> rendergraph::create_texture_views(rhi_device& device, rgtexture_id id)
	{
		texture_view_desc* viewdescs = m_texid_to_viewdesc_map[id];
		rhi_descriptor* descriptors = m_texid_to_descriptors_map[id];

		rgtexture* texture = get_texture(id);
		rhi_resource& resource = *texture->m_resource;
#if INFLUX_RG_BACKEND_RHI
		rhi::texture2D* texture_resource = (rhi::texture2D*)&resource;
#else 
		rhi_resource* texture_resource = &resource;
#endif

		// for each type of descriptor, allocate a cpu-handle and create the view
		for (uint8 i = 0u; i < k_num_descriptor_types; ++i)
		{
			if (viewdescs[i].m_is_active)
			{
				// allocate the handle on the CPU heap
				const rgdescriptor_type type = static_cast<rgdescriptor_type>(i);
				if (viewdescs[i].m_is_created == false)
				{
					auto alloc_result = m_pool->alloc_cpu_descriptor(type);
					if (alloc_result.is_fail())
					{
						return result<>::make_error("error: failed allocating a cpu handle!");
					}

					descriptors[i] = alloc_result.get();
				}

				// write the view onto the handle
				switch (type)
				{
				case rgdescriptor_type::render_target:
					device.create_rtv(texture_resource, descriptors[i]);
					break;
				case rgdescriptor_type::depth_target:
					device.create_dsv(texture_resource, descriptors[i]);
					break;
				case rgdescriptor_type::read_only:
					device.create_srv_texture(texture_resource, descriptors[i]);
					break;
				case rgdescriptor_type::read_write:
					if (resource.allows_uav() == false)
						return result<>::make_error("error: cannot create a uav for a texture that doesn't allow it!");
					
					device.create_uav_texture(texture_resource, descriptors[i]);
					break;
				}

				viewdescs[i].m_is_created = true;
			}
		}

		return {};
	}

	/* creates views (rtv/dsv/srv/samp) based on how the resource will be used in our rendergraph */
	result<> rendergraph::create_buffer_views(rhi_device& device, rgbuffer_id id)
	{
		auto& viewdescs = m_bufid_to_viewdesc_map[id];
		auto& descriptors = m_bufid_to_descriptors_map[id];

		rgbuffer* buffer = get_buffer(id);
#if INFLUX_RG_BACKEND_RHI
		rhi::buffer* buffer_resource = (rhi::buffer*)&buffer;
#endif

		for (uint8 i = 0u; i < k_num_descriptor_types; ++i)
		{
			if (viewdescs[i].m_is_active && viewdescs[i].m_is_created == false)
			{
				const rgdescriptor_type type = static_cast<rgdescriptor_type>(i);
				switch (type)
				{
				case rgdescriptor_type::read_only:
					descriptors[i] = m_pool->alloc_cpu_descriptor(type).get();
#if INFLUX_RG_BACKEND_RHI
					device.create_srv(*buffer_resource, descriptors[i]);
#else 
					device.create_buffer_srv(descriptors[i], buffer->m_resource);
#endif
					break;
				case rgdescriptor_type::read_write:
					descriptors[i] = m_pool->alloc_cpu_descriptor(type).get();
#if INFLUX_RG_BACKEND_RHI
					device.create_uav(*buffer_resource, descriptors[i]);
#else 
					device.create_buffer_uav(descriptors[i], buffer->m_resource);
#endif					break;

				default:
					return result<>::make_error("error: non-supported descriptor type for buffer!");
				}

				viewdescs[i].m_is_created = true;
			}
		}
		return {};
	}

	void rendergraph::build_adjacency()
	{
		m_adjacency_lists.resize(m_passes.size());
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			const rgpass& pass = m_passes[i];
			vector<uint64>& pass_adj_list = m_adjacency_lists[i];

			for (uint64 j = i + 1U; j < m_passes.size(); ++j)
			{
				const rgpass& other_pass = m_passes[j];
				if (other_pass.depends_on(pass))
				{
					pass_adj_list.push_back(j);
					break;
				}
			}
		}
	}

	void rendergraph::sort_topological()
	{
		vector<bool> visited_list(m_passes.size(), false);
		for (uint64 i = 0; i < m_passes.size(); i++)
		{
			if (visited_list[i] == false)
			{
				depth_search(i, visited_list, m_topo_sorted_passes);
			}
		}
		std::reverse(m_topo_sorted_passes.begin(), m_topo_sorted_passes.end());
	}

	void rendergraph::build_layers()
	{
		m_layers.clear();

		if (m_topo_sorted_passes.size() == 0u) return;

		vector<uint64> distances(m_topo_sorted_passes.size(), 0u);
		for (uint64 u = 0u; u < m_topo_sorted_passes.size(); ++u)
		{
			uint64 i = m_topo_sorted_passes[u];
			for (auto v : m_adjacency_lists[i])
			{
				if (distances[v] < distances[i] + 1u)
				{
					distances[v] = distances[i] + 1u;
				}
			}
		}

		const uint64 num_layers = *std::max_element(std::begin(distances), std::end(distances)) + 1u;
		m_layers.resize(num_layers, rglayer());
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			uint64 layer = distances[i];
			m_layers[layer].add_pass(m_passes[i]);
		}
	}

	void rendergraph::cull_passes()
	{
		for (rgpass& pass : m_passes)
		{
			pass.m_refcount = pass.get_num_writes();

			// if any of the resources are read, increase their refcount
			for (rgtexture_id id : pass.m_texture_reads)
			{
				rgtexture* texture = get_texture(id);
				texture->m_refcount += 1u;
			}
			for (rgbuffer_id id : pass.m_buffer_reads)
			{
				rgbuffer* buffer = get_buffer(id);
				buffer->m_refcount += 1u;
			}

			// if any of the resources are written, increase the refcount of the writer
			for (rgtexture_id id : pass.m_texture_writes)
			{
				rgtexture* written = get_texture(id);
				written->m_writer = &pass;
			}
			for (rgbuffer_id id : pass.m_buffer_writes)
			{
				rgbuffer* written = get_buffer(id);
				written->m_writer = &pass;
			}

			// gather & cull nullrefs
			std::stack<rgchild*> zero_ref_resources;
			for (auto& texture : m_textures) if (texture->m_refcount == 0) zero_ref_resources.push(texture);
			for (auto& buffer : m_buffers)   if (buffer->m_refcount == 0) zero_ref_resources.push(buffer);

			while (!zero_ref_resources.empty())
			{
				rgchild* resource = zero_ref_resources.top();
				zero_ref_resources.pop();

				rgpass* writer = resource->m_writer;
				if (writer == nullptr || !writer->can_be_culled())
				{
					continue;
				}

				--writer->m_refcount;
				if (writer->m_refcount == 0)
				{
					for (auto id : writer->m_texture_reads)
					{
						auto* texture = get_texture(id);
						if (--texture->m_refcount == 0) zero_ref_resources.push(texture);
					}
					for (auto id : writer->m_buffer_reads)
					{
						auto* buffer = get_buffer(id);
						if (--buffer->m_refcount == 0) zero_ref_resources.push(buffer);
					}
				}
			}

		}
	}

	void rendergraph::calc_resource_lifetimes()
	{
		// record last reads/writes
		for (rglayer& layer : m_layers)
		{
			for (rgpass& pass : layer.m_passes)
			{
				for (auto id : pass.m_texture_writes)
				{
					if (!pass.m_texture_state_map.contains(id)) continue;
					rgtexture* texture = get_texture(id);
					texture->m_final_pass = &pass;
				}
				for (auto id : pass.m_buffer_writes)
				{
					if (!pass.m_buffer_state_map.contains(id)) continue;
					rgbuffer* buffer = get_buffer(id);
					buffer->m_final_pass = &pass;
				}
				for (auto id : pass.m_texture_reads)
				{
					if (!pass.m_texture_state_map.contains(id)) continue;
					rgtexture* texture = get_texture(id);
					texture->m_final_pass = &pass;
				}
				for (auto id : pass.m_buffer_reads)
				{
					if (!pass.m_buffer_state_map.contains(id)) continue;
					rgbuffer* buffer = get_buffer(id);
					buffer->m_final_pass = &pass;
				}
			}
		}

		// insert destroy points at the final pass that uses the resource
		for (uint64 i = 0; i < m_textures.size(); ++i)
		{
			if (m_textures[i]->m_final_pass != nullptr && !m_textures[i]->is_imported())
				m_textures[i]->m_final_pass->m_texture_destroys.push_back(rgtexture_id(i));
		}
		for (uint64 i = 0; i < m_buffers.size(); ++i)
		{
			if (m_buffers[i]->m_final_pass != nullptr && !m_buffers[i]->is_imported())
			{
				m_buffers[i]->m_final_pass->m_buffer_destroys.push_back(rgbuffer_id(i));
			}
		}
		printf("");
	}

	void rendergraph::depth_search(uint64 parent_idx, vector<bool>& visited_list, vector<uint64>& topo_sorted_passes)
	{
		visited_list[parent_idx] = true;
		for (const auto& adj_idx : m_adjacency_lists[parent_idx])
		{
			if (visited_list[adj_idx] == false)
			{
				depth_search(adj_idx, visited_list, topo_sorted_passes);
			}
		}

		topo_sorted_passes.push_back(parent_idx);
	}


	// -- rgpass_builder uses these
	rgtexture_id rendergraph::declare_texture(const rgname& name, const texture_desc& desc)
	{
		if (!is_texture_declared(name))
		{
			rgtexture_id new_id = m_textures.size();
			rgtexture* new_texture = new rgtexture();

			new_texture->m_id = new_id;
			new_texture->m_desc = desc;
			new_texture->m_name = name;
			m_textures.push_back(new_texture);

			m_texture_name_to_id_map[name] = new_id;
			m_id_to_texture_map[new_id] = new_texture;
		}
		else
		{
			rgtexture_id id = m_texture_name_to_id_map[name];
			m_id_to_texture_map[id]->m_desc = desc;
		}

		return m_texture_name_to_id_map[name];
	}

	rgbuffer_id rendergraph::declare_buffer(const rgname& name, const buffer_desc& desc)
	{
		if (!is_buffer_declared(name))
		{
			rgbuffer_id new_id = m_buffers.size();
			rgbuffer* new_buffer = new rgbuffer();

			new_buffer->m_id = new_id;
			new_buffer->m_desc = desc;
			new_buffer->m_name = name;
			m_buffers.push_back(new_buffer);

			m_buffer_name_to_id_map[name] = new_id;
			m_id_to_buffer_map[new_id] = new_buffer;
		}
		else
		{
			rgbuffer_id id = m_buffer_name_to_id_map[name];
			m_id_to_buffer_map[id]->m_desc = desc;
		}

		return m_buffer_name_to_id_map[name];
	}

	bool rendergraph::is_texture_declared(rgtexture_id id) const
	{
		return m_id_to_texture_map.contains(id);
	}

	bool rendergraph::is_buffer_declared(rgbuffer_id id) const
	{
		return m_id_to_buffer_map.contains(id);
	}

	bool rendergraph::is_texture_declared(const rgname& name) const
	{
		return m_texture_name_to_id_map.contains(name);
	}

	bool rendergraph::is_buffer_declared(const rgname& name) const
	{
		return m_buffer_name_to_id_map.contains(name);
	}

	result<rgtex_copysrc_id> rendergraph::read_copysrc_texture(const rgname& name)
	{
		using result_type = result<rgtex_copysrc_id>;
		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		if (texture->m_desc.m_init_state == rhi_resource_state::common)
		{
			texture->m_desc.m_init_state = rhi_resource_state::copy_src;
		}
		return rgtex_copysrc_id(id);
	}

	result<rgtex_copydst_id> rendergraph::write_copydst_texture(const rgname& name)
	{
		using result_type = result<rgtex_copydst_id>;
		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		if (texture->m_desc.m_init_state == rhi_resource_state::common)
		{
			texture->m_desc.m_init_state = rhi_resource_state::copy_dst;
		}
		return rgtex_copydst_id(id);
	}

	result<rgbuf_copysrc_id> rendergraph::read_copysrc_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_copysrc_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_copysrc_id(id);
	}

	result<rgbuf_copydst_id> rendergraph::write_copydst_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_copydst_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_copydst_id(id);
	}

	result<rgbuf_indargs_id> rendergraph::read_indirect_args_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_indargs_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_indargs_id(id);
	}

	result<rgbuf_vertex_id> rendergraph::read_vertex_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_vertex_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_vertex_id(id);
	}

	result<rgbuf_index_id> rendergraph::read_index_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_index_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_index_id(id);
	}

	result<rgbuf_const_id> rendergraph::read_constant_buffer(const rgname& name)
	{
		using result_type = result<rgbuf_const_id>;
		if (m_buffer_name_to_id_map.contains(name) == false)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		return rgbuf_const_id(id);
	}

	result<rgid_rtv> rendergraph::rendertarget(const rgname& name, const texture_view_desc& view_desc)
	{
		using result_type = result<rgid_rtv>;
		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		texture_desc& desc = texture->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::rtv;
		if (desc.m_init_state == rhi_resource_state::common)
		{
			desc.m_init_state = rhi_resource_state::render_target;
		}

		// store the viewdesc
		texture_view_desc* viewdescs = m_texid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::render_target);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this texture already has an exact same view_desc, just return the existing one
			return rgid_rtv(desc_type_idx, id);
		}
		
		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_rtv(desc_type_idx, id);
	}

	result<rgid_dsv> rendergraph::depthtarget(const rgname& name, const texture_view_desc& view_desc)
	{
		using result_type = result<rgid_dsv>;

		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		texture_desc& desc = texture->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::dsv;
		if (desc.m_init_state == rhi_resource_state::common)
		{
			desc.m_init_state = rhi_resource_state::depth_target;
		}

		// store the viewdesc
		texture_view_desc* viewdescs = m_texid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::depth_target);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this texture already has an exact same view_desc, just return the existing one
			return rgid_dsv(desc_type_idx, id);
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_dsv(desc_type_idx, id);
	}

	result<rgid_srv_tex> rendergraph::read_texture(const rgname& name, const texture_view_desc& view_desc)
	{
		using result_type = result<rgid_srv_tex>;

		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		texture_desc& desc = texture->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::srv;
		if (desc.m_init_state == rhi_resource_state::common)
		{
			desc.m_init_state = rhi_resource_state::cs_srv | rhi_resource_state::ps_srv;
		}

		// store the viewdesc
		texture_view_desc* viewdescs = m_texid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::read_only);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this texture already has an exact same view_desc, just return the existing one
			return rgid_srv_tex(desc_type_idx, id);
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_srv_tex(desc_type_idx, id);
	}

	result<rgid_uav_tex> rendergraph::write_texture(const rgname& name, const texture_view_desc& view_desc)
	{
		using result_type = result<rgid_uav_tex>;

		rgtexture_id id = m_texture_name_to_id_map[name];
		rgtexture* texture = get_texture(id);
		if (texture == nullptr)
		{
			return result_type::make_error("texture by name not declared/imported!");
		}

		texture_desc& desc = texture->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::uav;
		if (desc.m_init_state == rhi_resource_state::common)
		{
			desc.m_init_state = rhi_resource_state::all_uav;
		}

		// store the viewdesc
		texture_view_desc* viewdescs = m_texid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::read_write);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this texture already has an exact same view_desc, just return the existing one
			return rgid_uav_tex(desc_type_idx, id);
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_uav_tex(desc_type_idx, id);
	}

	result<rgid_srv_buff> rendergraph::read_buffer(const rgname& name, const buffer_view_desc& view_desc)
	{
		using result_type = result<rgid_srv_buff>;

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		rgbuffer* buffer = get_buffer(id);
		if (buffer == nullptr)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		buffer_desc& desc = buffer->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::srv;

		// store the viewdesc
		buffer_view_desc* viewdescs = m_bufid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::read_only);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this buffer already has an exact same view_desc, just return the existing one
			return rgid_srv_buff(desc_type_idx, id);
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_srv_buff(desc_type_idx, id);
	}

	result<rgid_uav_buff> rendergraph::write_buffer(const rgname& name, const buffer_view_desc& view_desc)
	{
		using result_type = result<rgid_uav_buff>;
		
		rgbuffer_id id = m_buffer_name_to_id_map[name];
		rgbuffer* buffer = get_buffer(id);
		if (buffer == nullptr)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		buffer_desc& desc = buffer->m_desc;
		desc.m_bindflags |= rhi_resource_bindflags::uav;

		// store the viewdesc
		buffer_view_desc* viewdescs = m_bufid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::read_write);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			// if this buffer already has an exact same view_desc, just return the existing one
			return rgid_uav_buff(desc_type_idx, id);
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		return rgid_uav_buff(desc_type_idx, id);
	}

	result<rgid_uav_buff> rendergraph::write_buffer(const rgname& name, const rgname& counter_name, const buffer_view_desc& view_desc)
	{
		using result_type = result<rgid_uav_buff>;

		rgbuffer_id id = m_buffer_name_to_id_map[name];
		rgbuffer_id cnt_id = m_buffer_name_to_id_map[counter_name];

		rgbuffer* buffer = get_buffer(id);
		rgbuffer* cnt_buffer = get_buffer(id);
		if (buffer == nullptr)
		{
			return result_type::make_error("buffer by name not declared/imported!");
		}

		buffer_desc& desc = buffer->m_desc;
		buffer_desc& cnt_desc = cnt_buffer->m_desc;

		desc.m_bindflags |= rhi_resource_bindflags::uav;
		cnt_desc.m_bindflags |= rhi_resource_bindflags::uav;

		// store the viewdesc
		buffer_view_desc* viewdescs = m_bufid_to_viewdesc_map[id];
		const uint8 desc_type_idx = static_cast<uint8>(rgdescriptor_type::read_write);
		if (viewdescs[desc_type_idx].m_is_active && viewdescs[desc_type_idx] == view_desc)
		{
			auto readwrite_id = rgid_uav_buff(desc_type_idx, id);
			if (auto it = m_buffer_uav_counter_map.find(readwrite_id); it != m_buffer_uav_counter_map.end())
			{
				if (it->second == cnt_id) return readwrite_id;
			}
		}

		// create new if first time
		viewdescs[desc_type_idx] = view_desc;
		viewdescs[desc_type_idx].m_is_active = true;
		auto rw_id = rgid_uav_buff(desc_type_idx, id);
		m_buffer_uav_counter_map.insert(std::make_pair(rw_id, cnt_id));
		return rw_id;
	}

	rgtexture* rendergraph::get_texture(rgtexture_id id)
	{
		if (is_texture_declared(id))
		{
			return m_id_to_texture_map.at(id);
		}

		return nullptr;
	}

	rgbuffer* rendergraph::get_buffer(rgbuffer_id id)
	{
		if (is_buffer_declared(id))
		{
			return m_id_to_buffer_map.at(id);
		}

		return nullptr;
	}

	rgtexture_id rendergraph::get_texture_id(const rgname& name) const
	{
		return m_texture_name_to_id_map.at(name);
	}

	rgbuffer_id rendergraph::get_buffer_id(const rgname& name) const
	{
		return m_buffer_name_to_id_map.at(name);
	}

	texture_desc rendergraph::get_texture_desc(const rgname& name) const
	{
		rgtexture_id id = m_texture_name_to_id_map.at(name);
		rgtexture* texture = m_id_to_texture_map.at(id);
		return texture->m_desc;
	}

	buffer_desc rendergraph::get_buffer_desc(const rgname& name) const
	{
		rgbuffer_id id = m_buffer_name_to_id_map.at(name);
		rgbuffer* buffer = m_id_to_buffer_map.at(id);
		return buffer->m_desc;
	}

	bool rendergraph::execute_validation_checks() const
	{
		bool is_runnable = true;

		// any pass that has errors renders the validation!
		for (uint64 layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			const rglayer& layer = m_layers[layer_idx];
			for (uint64 pass_idx = 0u; pass_idx < layer.m_passes.size(); ++pass_idx)
			{
				const rgpass& pass = layer.m_passes[pass_idx];
				if (pass.m_num_errors != 0u)
				{
					return false;
				}
			}
		}

		// imported resources with a pending uav create should allow for UAV!
		bool uav_check = true;
		uint32 num_incorrect_uavs = 0u;
		{
			constexpr uint32 uav_index = static_cast<uint32>(rgdescriptor_type::read_write);
			for (uint64 i = 0; i < m_textures.size(); ++i)
			{
				if (m_textures[i]->m_is_imported)
				{
					rgtexture_id id = m_textures[i]->m_id;
					const rhi_resource& resource = *m_textures[i]->m_resource;
					if (m_texid_to_viewdesc_map.contains(id))
					{
						const bool wants_uav = m_texid_to_viewdesc_map.at(id)[uav_index].m_is_active;
						if (wants_uav && resource.allows_uav() == false)
						{
							++num_incorrect_uavs;
						}
					}
				}
			}
			for (uint64 i = 0; i < m_buffers.size(); ++i)
			{
				if (m_buffers[i]->m_is_imported)
				{
					rgbuffer_id id = m_buffers[i]->m_id;
					const rhi_resource& resource = *m_buffers[i]->m_resource;
					if (m_bufid_to_viewdesc_map.contains(id))
					{
						const bool wants_uav = m_bufid_to_viewdesc_map.at(id)[uav_index].m_is_active;
						if (wants_uav && resource.allows_uav() == false)
						{
							++num_incorrect_uavs;
						}
					}
				}
			}
			uav_check = num_incorrect_uavs == 0u;
		}

		if (!uav_check)
		{
			logonce(e_log_category::warning, "rendergraph::validate() >> {} imported resources are marked readwrite (uav) but are created as non-uav compatible!", 
				num_incorrect_uavs);

			is_runnable = false;
		}
		
		return is_runnable;
	}

	rhi_descriptor rendergraph::get_rtv(rgtexture_id id)
	{
		influx_assert(m_texid_to_descriptors_map.contains(id));
		return m_texid_to_descriptors_map[id][static_cast<uint32>(rgdescriptor_type::render_target)];
	}

	rhi_descriptor rendergraph::get_dsv(rgtexture_id id)
	{
		influx_assert(m_texid_to_descriptors_map.contains(id));
		return m_texid_to_descriptors_map[id][static_cast<uint32>(rgdescriptor_type::depth_target)];
	}

	rhi_descriptor rendergraph::get_readonly(rgtexture_id id)
	{
		influx_assert(m_texid_to_descriptors_map.contains(id));
		return m_texid_to_descriptors_map[id][static_cast<uint32>(rgdescriptor_type::read_only)];
	}

	rhi_descriptor rendergraph::get_readwrite(rgtexture_id id)
	{
		influx_assert(m_texid_to_descriptors_map.contains(id));
		return m_texid_to_descriptors_map[id][static_cast<uint32>(rgdescriptor_type::read_write)];
	}

#pragma region rgpass_context
	result<rgpass_context::resource_and_view> rgpass_context::get_copysrc(rgtex_copysrc_id id)
	{
		resource_and_view result{};
		rgtexture_id res_id = rgtexture_id(id);
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copysrc(rgbuf_copysrc_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copydst(rgtex_copydst_id id)
	{
		resource_and_view result{};
		rgtexture_id res_id = rgtexture_id(id);
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copydst(rgbuf_copydst_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_vertexbuffer(rgbuf_vertex_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_indexbuffer(rgbuf_index_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_constbuffer(rgbuf_const_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_indirect_args_resource(rgbuf_indargs_id id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = rgbuffer_id(id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copysrc_texture(const rgname& name)
	{
		resource_and_view result{};
		rgtexture_id res_id = m_graph.m_texture_name_to_id_map[name];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copysrc_buffer(const rgname& name)
	{
		resource_and_view result{};
		rgbuffer_id res_id = m_graph.m_buffer_name_to_id_map[name];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copydst_texture(const rgname& name)
	{
		resource_and_view result{};
		rgtexture_id res_id = m_graph.m_texture_name_to_id_map[name];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_copydst_buffer(const rgname& name)
	{
		resource_and_view result{};
		rgbuffer_id res_id = m_graph.m_buffer_name_to_id_map[name];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_constbuffer(const rgname& name)
	{
		resource_and_view result{};
		rgbuffer_id res_id = m_graph.m_buffer_name_to_id_map[name];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		result.m_descriptor = 0u;
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_rtv(uint32 at_index)
	{
		using result_type = result<rgpass_context::resource_and_view>;
		const uint64 num_rtvs = m_pass.m_rtvs.size();
		if (at_index >= num_rtvs)
			return result_type::make_error("error: this pass has no render target at this index!");

		resource_and_view result{};
		rgtexture_id res_id = m_pass.m_rtvs[at_index].m_texture_id;
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::render_target)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_dsv()
	{
		using result_type = result<rgpass_context::resource_and_view>;

		if (m_pass.m_dsv.m_is_enabled == false)
			return result_type::make_error("error: this pass has no depth target!");

		resource_and_view result{};
		rgtexture_id res_id = m_pass.m_dsv.m_texture_id;
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::depth_target)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_rtv(rgid_rtv id)
	{
		resource_and_view result{};
		rgtexture_id res_id = id.get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::render_target)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_dsv(rgid_rtv id)
	{
		resource_and_view result{};
		rgtexture_id res_id = id.get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::depth_target)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_texture(rgid_srv_tex id)
	{
		resource_and_view result{};
		rgtexture_id res_id = id.get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_texture(rgid_uav_tex id)
	{
		resource_and_view result{};
		rgtexture_id res_id = id.get_resource_id();
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_resource = nullptr;
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_buffer(rgid_srv_buff id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = id.get_resource_id();
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_buffer(rgid_uav_buff id)
	{
		resource_and_view result{};
		rgbuffer_id res_id = id.get_resource_id();
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_texture(const rgname& name)
	{
		resource_and_view result{};
		rgtexture_id res_id = m_graph.m_texture_name_to_id_map[name];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_texture(const rgname& name)
	{
		resource_and_view result{};
		rgtexture_id res_id = m_graph.m_texture_name_to_id_map[name];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_buffer(const rgname& name)
	{
		resource_and_view result{};
		rgbuffer_id res_id = m_graph.m_buffer_name_to_id_map[name];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_buffer(const rgname& name)
	{
		resource_and_view result{};
		rgbuffer_id res_id = m_graph.m_buffer_name_to_id_map[name];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_texture(uint32 index)
	{
		using result_type = result<rgpass_context::resource_and_view>;
		if (index >= m_pass.m_texture_reads.size())
			return result_type::make_error("error: index out of bounds!");

		resource_and_view result{};
		rgtexture_id res_id = m_pass.m_texture_reads[index];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_texture(uint32 index)
	{
		using result_type = result<rgpass_context::resource_and_view>;
		if (index >= m_pass.m_texture_writes.size())
			return result_type::make_error("error: index out of bounds!");

		resource_and_view result{};
		rgtexture_id res_id = m_pass.m_texture_writes[index];
		rgtexture* texture = m_graph.get_texture(res_id);
		result.m_resource = texture->m_resource;
		const auto& views = m_graph.m_texid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_read_buffer(uint32 index)
	{
		using result_type = result<rgpass_context::resource_and_view>;
		if (index >= m_pass.m_buffer_reads.size())
			return result_type::make_error("error: index out of bounds!");

		resource_and_view result{};
		rgbuffer_id res_id = m_pass.m_buffer_reads[index];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_only)];
		return result;
	}
	result<rgpass_context::resource_and_view> rgpass_context::get_write_buffer(uint32 index)
	{
		using result_type = result<rgpass_context::resource_and_view>;
		if (index >= m_pass.m_buffer_writes.size())
			return result_type::make_error("error: index out of bounds!");

		resource_and_view result{};
		rgbuffer_id res_id = m_pass.m_buffer_writes[index];
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		result.m_resource = buffer->m_resource;
		const auto& views = m_graph.m_bufid_to_descriptors_map[res_id];
		result.m_descriptor = views[static_cast<uint32>(rgdescriptor_type::read_write)];
		return result;
	}
	rhi_descheap& rgpass_context::get_descheap_gpu(e_gpu_descheap slot)
	{
		return *m_graph.m_pool->get_gpu_descheap(slot);
	}
#pragma endregion
}

