#include "renderer_pch.h"

#include "influx_renderer/rendergraph/rendergraph.h"
#include "influx_renderer/rendergraph/rgpass.h"
#include "influx_renderer/rendergraph/rgpool.h"
#include "influx_renderer/rendergraph/rgresources.h"

#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

#include "core/enum.h" // has_any_flag()...

namespace influx::renderer
{
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

	class rglayer final
	{
	public:
		rglayer() = default;

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

		vector<rgpass_base*> m_passes;

		vector<rgtexture_id> m_texture_creates;
		vector<rgtexture_id> m_texture_reads;
		vector<rgtexture_id> m_texture_writes;
		vector<rgtexture_id> m_texture_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_texture_to_state_map;
		
		vector<rgbuffer_id> m_buffer_creates;
		vector<rgbuffer_id> m_buffer_reads;
		vector<rgbuffer_id> m_buffer_writes;
		vector<rgbuffer_id> m_buffer_destroys;
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
		build_layers();

		// todo: cull passes
		// ...
		
		// todo: calc resource lifetimes
		// ...

		for (const rglayer* layer : m_layers)
		{
			for (rgpass_base* pass : layer->m_passes)
			{
				pass->setup();
			}
		}
	}

	void rendergraph::execute(graphics::commandlist* commandlist)
	{
		m_pool->tick();

		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			rglayer const* layer = m_layers[layer_idx];

			// texture creates
			for (const rgtexture_id& tex_id : layer->m_texture_creates)
			{
				rgtexture* texture = get_texture(tex_id);
				texture->m_resource; // todo: allocate from pool
				// todo: create descriptors
				// todo: set name
			}

			// buffer creates
			for (const rgbuffer_id& buff_id : layer->m_buffer_creates)
			{
				rgbuffer* buffer = get_buffer(buff_id);
				buffer->m_resource; // todo: allocate from pool
				// todo: create descriptors
				// todo: set name
			}

			// todo: texture transitions

			// todo: buffer transitions
			
			// todo: flush barriers

			// todo: run each pass in the layer
			graphics::commandlist* cmdlist = nullptr;
			for (size_t pass_idx = 0u; pass_idx < layer->m_passes.size(); ++pass_idx)
			{
				rgpass_base* pass = layer->m_passes[pass_idx];

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

						graphics::descriptor_handle* handle = get_rtv(rtv.m_texture_id);
						influx_assert(handle);
						color_attachment.m_rtv_descriptor = handle;
					}

					// dsv
					{
						auto& dsv = pass->m_dsv;
						auto& depth_attachment = args.m_depth_attachment;

						depth_attachment.m_depth_load = translate(dsv.m_depth_access.m_load);
						depth_attachment.m_depth_store = translate(dsv.m_depth_access.m_store);
						depth_attachment.m_stencil_load = translate(dsv.m_stencil_access.m_load);
						depth_attachment.m_stencil_store = translate(dsv.m_stencil_access.m_store);

						depth_attachment.m_depth_clear;
						depth_attachment.m_stencil_clear;

						graphics::descriptor_handle* handle = get_dsv(dsv.m_texture_id);
						influx_assert(handle);
						depth_attachment.m_dsv_descriptor = handle;
					}

					{
						influx_scope("renderpass");
						cmdlist->renderpass_begin(args);
						pass->execute();
						cmdlist->renderpass_end();
					}
				}
			}

			// texture destroys
			for (const rgtexture_id& tex_id : layer->m_texture_destroys)
			{
				rgtexture* texture = get_texture(tex_id);
				texture->m_resource; // todo: de-allocate from pool
			}

			// buffer destroys
			for (const rgbuffer_id& buff_id : layer->m_buffer_destroys)
			{
				rgbuffer* buffer = get_buffer(buff_id);
				buffer->m_resource; // todo: de-allocate from pool
			}
		}
	}

	void rendergraph::build_adjacency()
	{
		m_adjacency_lists.resize(m_passes.size());
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			rgpass_base* pass = m_passes[i];
			vector<uint64>& pass_adj_list = m_adjacency_lists[i];

			for (uint64 j = i + 1U; j < m_passes.size(); ++j)
			{
				rgpass_base* other_pass = m_passes[j];
				bool dependency = rgpass_base::has_dependency(pass, other_pass);
				if (dependency)
				{
					pass_adj_list.push_back(j);
					break;
				}
			}
		}
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
			uint64 layer = distances[i];
			m_layers[layer].push_back(m_passes[i]); // add the pass to the layer
		}
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

	rgtexture_id rendergraph::import_texture(const rgname& name, texture* texture)
	{
		rgtexture* new_texture = new rgtexture();
		m_textures.push_back(new_texture);

		rgtexture_id new_id = m_textures.size() - 1u;
		new_texture->m_id = new_id;
		new_texture->m_is_imported = true;

		m_imported_texture_map[texture] = new_texture;

		return new_id;
	}

	rgtexture_id rendergraph::import_buffer(const rgname& name, buffer* buffer)
	{
		rgbuffer* new_buffer = new rgbuffer();
		m_buffers.push_back(new_buffer);

		rgbuffer_id new_id = m_buffers.size() - 1u;
		new_buffer->m_id = new_id;
		new_buffer->m_is_imported = true;

		// register import
		m_imported_buffer_map[buffer] = new_buffer;

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

	rgpass_base* rendergraph::get_pass(rgpass_id id)
	{
		if (is_pass_declared(id))
		{
			return m_id_to_pass_map[id];
		}

		return nullptr;
	}

	graphics::descriptor_handle* rendergraph::get_rtv(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::rendertarget)];
	}

	graphics::descriptor_handle* rendergraph::get_dsv(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::depthstencil)];
	}

	graphics::descriptor_handle* rendergraph::get_readonly(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readonly)];
	}

	graphics::descriptor_handle* rendergraph::get_readonly(rgbuffer_id id)
	{
		influx_assert(m_buffer_to_descriptors_map.contains(id));
		m_buffer_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readonly)];
	}

	graphics::descriptor_handle* rendergraph::get_readwrite(rgbuffer_id id)
	{
		influx_assert(m_buffer_to_descriptors_map.contains(id));
		m_buffer_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readwrite)];
	}

	graphics::descriptor_handle* rendergraph::get_readwrite(rgtexture_id id)
	{
		influx_assert(m_texture_to_descriptors_map.contains(id));
		return m_texture_to_descriptors_map[id][static_cast<uint32>(e_descriptor_type::readwrite)];
	}
}

