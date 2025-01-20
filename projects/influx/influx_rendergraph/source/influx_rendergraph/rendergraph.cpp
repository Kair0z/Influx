#include "rendergraph_pch.h"

// influx::core
#include "core/scope.h"
#include "core/enum.h" // has_any_flag()...

// influx::rendergraph
#include "rendergraph.h"
#include "rgpass.h"
#include "rgpool.h"
#include "rgresources.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

// stl
#include <stack>

namespace influx::rendergraph
{
#pragma region translation
	constexpr graphics::e_load_op translate(const e_rg_load load)
	{
		switch (load)
		{
		case e_rg_load::clear: return graphics::e_load_op::clear;
		case e_rg_load::discard: return graphics::e_load_op::discard;
		case e_rg_load::preserve: return graphics::e_load_op::preserve;
		case e_rg_load::no_access: return graphics::e_load_op::no_access;
		}
		return graphics::e_load_op::count;
	}

	constexpr graphics::e_store_op translate(const e_rg_store store)
	{
		switch (store)
		{
		case e_rg_store::resolve: return graphics::e_store_op::resolve;
		case e_rg_store::discard: return graphics::e_store_op::discard;
		case e_rg_store::preserve: return graphics::e_store_op::preserve;
		case e_rg_store::no_access: return graphics::e_store_op::no_access;
		}
		return graphics::e_store_op::count;
	}
#pragma endregion

	class rglayer final
	{
	public:
		rglayer() = default;
		~rglayer() = default;

		inline void setup()
		{
			for (rgpass* pass : m_passes)
			{
				if (pass->is_culled()) continue;

				m_texture_creates.insert(pass->m_texture_creates.begin(), pass->m_texture_creates.end());
				m_texture_destroys.insert(pass->m_texture_destroys.begin(), pass->m_texture_destroys.end());
				for (auto [resource, state] : pass->m_texture_state_map)
				{
					m_texture_to_state_map[resource] |= state;
				}

				m_buffer_creates.insert(pass->m_buffer_creates.begin(), pass->m_buffer_creates.end());
				m_buffer_destroys.insert(pass->m_buffer_destroys.begin(), pass->m_buffer_destroys.end());
				for (auto [resource, state] : pass->m_buffer_state_map)
				{
					m_buffer_to_state_map[resource] |= state;
				}
			}
		}

		inline void reset()
		{
			m_passes.clear();
			m_texture_creates.clear();
			m_texture_reads.clear();
			m_texture_writes.clear();
			m_texture_destroys.clear();
			m_texture_to_state_map.clear();
			m_buffer_creates.clear();
			m_buffer_reads.clear();
			m_buffer_writes.clear();
			m_buffer_destroys.clear();
			m_buffer_to_state_map.clear();
		}

		vector<rgpass*> m_passes;

		uset<rgtexture_id> m_texture_creates;
		uset<rgtexture_id> m_texture_reads;
		uset<rgtexture_id> m_texture_writes;
		uset<rgtexture_id> m_texture_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_texture_to_state_map;
		
		uset<rgbuffer_id> m_buffer_creates;
		uset<rgbuffer_id> m_buffer_reads;
		uset<rgbuffer_id> m_buffer_writes;
		uset<rgbuffer_id> m_buffer_destroys;
		umap<rgbuffer_id, graphics::e_resource_state> m_buffer_to_state_map;
	};

	class gpu_view_manager final
	{
	public:
		gpu_view_manager(graphics::device* device)
		{
			graphics::descriptor_heap::create_args args{};
			args.m_shader_visible = true;

			args.m_capacity = 8u;
			args.m_type = graphics::e_descriptor_heap_type::sampler;
			m_sampler_heap = device->create_descriptor_heap(args);

			args.m_capacity = 64u;
			args.m_type = graphics::e_descriptor_heap_type::srv;
			m_resource_heap = device->create_descriptor_heap(args);
		}

		graphics::descriptor_handle alloc_gpu_resource()
		{
			return m_resource_heap->allocate_gpu();
		}

		graphics::descriptor_handle alloc_gpu_sampler()
		{
			return m_sampler_heap->allocate_gpu();
		}

		void end_frame()
		{
			m_resource_heap->free_all_gpu();
			m_resource_heap->free_all_cpu();
			m_sampler_heap->free_all_cpu();
			m_sampler_heap->free_all_gpu();
		}

	private:
		graphics::descriptor_heap* m_resource_heap;
		graphics::descriptor_heap* m_sampler_heap;
	};

	rendergraph::rendergraph(graphics::device* device)
		: m_device{ device }
	{
		m_pool = new rgpool(device);
		m_view_manager = new gpu_view_manager(device);
	}
	
	void rendergraph::build()
	{
		build_adjacency();
		sort_topological();
		build_layers();
		cull_passes();
		calc_resource_lifetimes();

		for (rglayer* layer : m_layers)
		{
			layer->setup();
		}
	}

	void rendergraph::execute(graphics::commandlist* commandlist)
	{
		m_pool->tick();

		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			const rglayer& layer = *m_layers[layer_idx];

			// creates
			{
				for (const rgtexture_id& tex_id : layer.m_texture_creates)
				{
					rgtexture* texture = get_texture(tex_id);
					texture->m_resource = m_pool->allocate_texture_resource(texture->m_desc);
					create_texture_views(tex_id);
					// todo: set name
				}
				for (const rgbuffer_id& buff_id : layer.m_buffer_creates)
				{
					rgbuffer* buffer = get_buffer(buff_id);
					buffer->m_resource = m_pool->allocate_buffer_resource(buffer->m_desc);
					create_buffer_views(buff_id);
					// todo: set name
				}
			}

			// transitions
			{
#if 0
				for (auto const& [tex_id, state] : layer->m_texture_to_state_map)
				{
					rgtexture* texture = get_texture(tex_id);
					graphics::resource* resource = nullptr; // ...

					// if this texture is freshly created, use initial state
					if (layer->m_texture_creates.contains(tex_id))
					{
						if (!has_all_flags(texture - ().initial_state, state))
						{
							commandlist->texture_barrier(resource, texture->GetDesc().initial_state, state);
						}
						continue;
					}

					bool found = false;
#if 0
					for (int j = (int)i - 1; j >= 0; --j)
					{
						auto& prev_dependency_level = dependency_levels[j];
						if (prev_dependency_level.texture_state_map.contains(tex_id))
						{
							GfxResourceState prev_state = prev_dependency_level.texture_state_map[tex_id];
							if (prev_state != state) cmd_list->TextureBarrier(*texture, prev_state, state);
							found = true;
							break;
						}
					}
#endif

					// if it's not in our graph, and it's imported
					if (!found && texture->m_is_imported)
					{
						GfxResourceState prev_state = rg_texture->desc.initial_state;
						if (prev_state != state) cmd_list->TextureBarrier(*texture, prev_state, state);
					}
				}
				for (auto const& [buf_id, state] : dependency_level.buffer_state_map)
				{
					RGBuffer* rg_buffer = GetRGBuffer(buf_id);
					GfxBuffer* buffer = rg_buffer->resource;
					if (dependency_level.buffer_creates.contains(buf_id))
					{
						if (state != GfxResourceState::Common)
						{
							cmd_list->BufferBarrier(*buffer, GfxResourceState::Common, state);
						}
						continue;
					}
					Bool found = false;
					for (Int32 j = (Int32)i - 1; j >= 0; --j)
					{
						auto& prev_dependency_level = dependency_levels[j];
						if (prev_dependency_level.buffer_state_map.contains(buf_id))
						{
							GfxResourceState prev_state = prev_dependency_level.buffer_state_map[buf_id];
							if (prev_state != state) cmd_list->BufferBarrier(*buffer, prev_state, state);
							found = true;
							break;
						}
					}
					if (!found && rg_buffer->imported)
					{
						if (GfxResourceState::Common != state) cmd_list->BufferBarrier(*buffer, GfxResourceState::Common, state);
					}
				}
#endif
				commandlist->flush_barriers();
			}

			// execute passes
			{
				for (size_t pass_idx = 0u; pass_idx < layer.m_passes.size(); ++pass_idx)
				{
					rgpass* pass = layer.m_passes[pass_idx];

					if (pass->is_culled())
					{
						continue;
					}

					if (pass->get_type() == e_rgpass_type::graphics)
					{
						graphics::renderpass_args args{};
						args.m_width = pass->get_width();
						args.m_height = pass->get_height();
						args.m_legacy = false;

						// rtvs
						args.m_color_attachments.reserve(pass->m_rtvs.size());
						for (const auto& rtv : pass->m_rtvs)
						{
							args.m_color_attachments.push_back({});
							auto& color_attachment = args.m_color_attachments.back();

							color_attachment.m_load = translate(rtv.m_access.m_load);
							color_attachment.m_store = translate(rtv.m_access.m_store);

							rgtexture* color_texture = get_texture(rtv.m_texture_id);
							influx_assert(color_texture != nullptr);

							color_attachment.m_clear;

							graphics::descriptor_handle handle = get_rtv(rtv.m_texture_id);
							influx_assert(handle);
							color_attachment.m_rtv_descriptor = handle;
						}

						// dsv
						if (false)
						{
							auto& dsv = pass->m_dsv;
							auto& depth_attachment = args.m_depth_attachment;

							depth_attachment.m_depth_load = translate(dsv.m_depth_access.m_load);
							depth_attachment.m_depth_store = translate(dsv.m_depth_access.m_store);
							depth_attachment.m_stencil_load = translate(dsv.m_stencil_access.m_load);
							depth_attachment.m_stencil_store = translate(dsv.m_stencil_access.m_store);

							depth_attachment.m_depth_clear;
							depth_attachment.m_stencil_clear;

							graphics::descriptor_handle handle = get_dsv(dsv.m_texture_id);
							influx_assert(handle);
							depth_attachment.m_dsv_descriptor = handle;
						}

						{
							rgpass_context ctx{};

							influx_scope("renderpass");
							commandlist->renderpass_begin(args);
							pass->execute(ctx);
							commandlist->renderpass_end();
						}
					}
				}
			}

			// execute destroys
			{
				for (const rgtexture_id& tex_id : layer.m_texture_destroys)
				{
					rgtexture* texture = get_texture(tex_id);
					m_pool->release_texture(texture->m_resource);
				}
				for (const rgbuffer_id& buff_id : layer.m_buffer_destroys)
				{
					rgbuffer* buffer = get_buffer(buff_id);
					m_pool->release_buffer(buffer->m_resource);
				}
			}
		}
	}

	rgpass* rendergraph::add_pass(const rgpass_callback& callback)
	{
		rgpass* new_pass = new rgpass(callback);
		m_passes.emplace_back(new_pass);

		rgpass_id new_id = m_passes.size() - 1u;
		new_pass->set_id(new_id);

		m_id_to_pass_map[new_id] = new_pass;

		return new_pass;
	}

	void rendergraph::create_texture_views(rgtexture_id)
	{
		// todo
		influx_assert(false);
	}

	void rendergraph::create_buffer_views(rgbuffer_id)
	{
		// todo
		influx_assert(false);
	}

	void rendergraph::build_adjacency()
	{
		m_adjacency_lists.resize(m_passes.size());
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			rgpass* pass = m_passes[i];
			vector<uint64>& pass_adj_list = m_adjacency_lists[i];

			for (uint64 j = i + 1U; j < m_passes.size(); ++j)
			{
				rgpass* other_pass = m_passes[j];
				bool dependency = rgpass::has_dependency(pass, other_pass);
				if (dependency)
				{
					pass_adj_list.push_back(j);
					break;
				}
			}
		}
	}

	void rendergraph::sort_topological()
	{
		vector<bool>  visited_list(m_passes.size(), false);
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
		vector<uint64> distances(m_passes.size(), 0u);
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			for (auto v : m_adjacency_lists[i])
			{
				if (distances[v] < distances[i] + 1u)
				{
					distances[v] = distances[i] + 1u;
				}
			}
		}

		const uint64 num_layers = *std::max_element(std::begin(distances), std::end(distances)) + 1u;
		m_layers.resize(num_layers);

		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			if (m_layers[i] == nullptr)
			{
				m_layers[i] = new rglayer();
			}

			uint64 layer = distances[i];
			m_layers[layer]->m_passes.push_back(m_passes[i]); // add the pass to the layer
		}
	}

	void rendergraph::cull_passes()
	{
		for (rgpass* pass : m_passes)
		{
			pass->m_refcount = pass->get_num_writes();

			// if any of the resources are read, increase their refcount
			for (rgtexture_id id : pass->m_texture_reads)
			{
				rgtexture* texture = get_texture(id);
				texture->m_refcount += 1u;
			}
			for (rgbuffer_id id : pass->m_buffer_reads)
			{
				rgbuffer* buffer = get_buffer(id);
				buffer->m_refcount += 1u;
			}

			// if any of the resources are written, increase the refcount of the writer
			for (rgtexture_id id : pass->m_texture_writes)
			{
				rgtexture* written = get_texture(id);
				written->m_writer = pass;
			}
			for (rgbuffer_id id : pass->m_buffer_writes)
			{
				rgbuffer* written = get_buffer(id);
				written->m_writer = pass;
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
		for (rglayer* layer : m_layers)
		{
			for (rgpass* pass : layer->m_passes)
			{
				if (pass->is_culled())
				{
					continue;
				}

				for (auto id : pass->m_texture_writes)
				{
					if (!pass->m_texture_state_map.contains(id)) continue;
					rgtexture* texture = get_texture(id);
					texture->m_last_user = pass;
				}
				for (auto id : pass->m_buffer_writes)
				{
					if (!pass->m_buffer_state_map.contains(id)) continue;
					rgbuffer* buffer = get_buffer(id);
					buffer->m_last_user = pass;
				}
				for (auto id : pass->m_texture_reads)
				{
					if (!pass->m_texture_state_map.contains(id)) continue;
					rgtexture* texture = get_texture(id);
					texture->m_last_user = pass;
				}
				for (auto id : pass->m_buffer_reads)
				{
					if (!pass->m_buffer_state_map.contains(id)) continue;
					rgbuffer* buffer = get_buffer(id);
					buffer->m_last_user = pass;
				}
			}
		}

		// insert destroy points at the 'last user pass' of the resources
		for (uint64 i = 0; i < m_textures.size(); ++i)
		{
			if (m_textures[i]->m_last_user != nullptr) m_textures[i]->m_last_user->m_texture_destroys.insert(rgtexture_id(i));
			if (m_textures[i]->m_is_imported) create_texture_views(rgtexture_id(i));
		}
		for (uint64 i = 0; i < m_buffers.size(); ++i)
		{
			if (m_buffers[i]->m_last_user != nullptr) m_buffers[i]->m_last_user->m_buffer_destroys.insert(rgbuffer_id(i));
			if (m_buffers[i]->m_is_imported) create_buffer_views(rgbuffer_id(i));
		}
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

	rgtexture_id rendergraph::add_texture(const rgname& name, const texture_desc& desc)
	{
		rgtexture* new_texture = new rgtexture();
		m_textures.push_back(new_texture);

		rgtexture_id new_id = m_textures.size() - 1u;
		new_texture->m_id = new_id;

		return new_id;
	}

	rgbuffer_id rendergraph::add_buffer(const rgname& name, const buffer_desc& desc)
	{
		rgbuffer* new_buffer = new rgbuffer();
		m_buffers.push_back(new_buffer);

		rgbuffer_id new_id = m_buffers.size() - 1u;
		new_buffer->m_id = new_id;

		return new_id;
	}

	bool rendergraph::is_texture_declared(rgtexture_id id) const
	{
		return m_id_to_texture_map.contains(id);
	}

	bool rendergraph::is_buffer_declared(rgbuffer_id id) const
	{
		return m_id_to_buffer_map.contains(id);
	}

	bool rendergraph::is_pass_declared(rgpass_id id) const
	{
		return m_id_to_pass_map.contains(id);
	}

	rgtexture* rendergraph::get_texture(rgtexture_id id)
	{
		if (is_texture_declared(id))
		{
			return m_id_to_texture_map[id];
		}

		return nullptr;
	}

	rgbuffer* rendergraph::get_buffer(rgbuffer_id id)
	{
		if (is_buffer_declared(id))
		{
			return m_id_to_buffer_map[id];
		}

		return nullptr;
	}

	rgpass* rendergraph::get_pass(rgpass_id id)
	{
		if (is_pass_declared(id))
		{
			return m_id_to_pass_map[id];
		}

		return nullptr;
	}

	graphics::descriptor_handle rendergraph::get_rtv(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::rendertarget)];
	}

	graphics::descriptor_handle rendergraph::get_dsv(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::depthstencil)];
	}

	graphics::descriptor_handle rendergraph::get_readonly(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readonly)];
	}

	graphics::descriptor_handle rendergraph::get_readwrite(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readwrite)];
	}
}

