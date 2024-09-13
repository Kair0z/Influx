#include "renderer_pch.h"

#include "influx_renderer/rendergraph/rendergraph.h"
#include "influx_renderer/rendergraph/rgpass.h"
#include "influx_renderer/rendergraph/rgpool.h"
#include "influx_renderer/rendergraph/rgresources.h"

#include "influx_graphics/commandlist.h"

#include "core/enum.h" // has_any_flag()...

namespace influx::renderer
{
	rendergraph::rendergraph(graphics::device* device)
		: m_device{ device }
	{
		m_pool = new rgpool();
	}
	
	void rendergraph::build()
	{
		build_adjacency();
		build_layers();

		// todo: cull passes
		// ...
		
		// todo: calc resource lifetimes
		// ...

		for (const rglayer& layer : m_layers)
		{
			for (rgpass_base* pass : layer)
			{
				pass->setup();
			}
		}
	}

	void rendergraph::execute(graphics::command_list* commandlist)
	{
		m_pool->tick();

		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			const rglayer& layer = m_layers[layer_idx];

			// todo: texture creates

			// todo: buffer creates

			// todo: texture transitions

			// todo: buffer transitions

			// todo: run each pass in the layer
			graphics::command_list* cmdlist = nullptr;
			for (size_t pass_idx = 0u; pass_idx < m_passes.size(); ++pass_idx)
			{
				rgpass_base* pass = m_passes[pass_idx];
				if (pass->get_type() == e_rgpass_type::graphics)
				{
					// todo: allow uav writes?
					// ...

					// todo: profiling scope
					// ...

					graphics::renderpass_args args{};
					cmdlist->renderpass_begin(args);
					pass->execute();
					cmdlist->renderpass_end();
				}
			}

			// todo: texture destroys

			// todo: buffer destroys
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
}

