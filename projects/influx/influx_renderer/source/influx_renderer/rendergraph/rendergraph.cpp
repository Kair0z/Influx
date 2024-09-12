#include "renderer_pch.h"

#include "influx_renderer/rendergraph/rendergraph.h"
#include "influx_renderer/rendergraph/rgpass.h"
#include "influx_renderer/rendergraph/rgpool.h"

#include "influx_graphics/commandlist.h"

#include "core/enum.h" // has_any_flag()...

namespace influx::renderer
{
	class rglayer final
	{
	private:
		friend class rendergraph;
		rglayer(rendergraph& rg) : m_rg{ rg } {}

		void setup();
		void execute();

		rendergraph& m_rg;
		vector<rgpass_base*> m_passes;
	};

	rendergraph::rendergraph(graphics::device* device)
	{
		m_pool = new rgpool();
	}
	
	void rendergraph::build()
	{
		build_adjacency();
		build_layers();

		// cull passes
		// calc resource lifetimes

		for (rglayer* layer : m_layers)
		{
			layer->setup();
		}
	}

	void rendergraph::execute(graphics::command_list* commandlist)
	{
		m_pool->tick();

		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			rglayer* layer = m_layers[layer_idx];

			// texture creates

			// buffer creates

			// texture transitions

			// buffer transitions

			// execute
			layer->execute();

			// texture destroys

			// buffer destroys
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

		m_layers.resize(*std::max_element(std::begin(distances), std::end(distances)) + 1u, new rglayer(*this));
		for (uint64 i = 0u; i < m_passes.size(); ++i)
		{
			uint64 layer = distances[i];
			m_layers[layer]; // add_pass();
		}
	}

#pragma region rglayer
	void rglayer::setup()
	{
		// todo
	}

	void rglayer::execute()
	{
		graphics::command_list* cmdlist = nullptr;
		for (size_t pass_idx = 0u; pass_idx < m_passes.size(); ++pass_idx)
		{
			rgpass_base* pass = m_passes[pass_idx];
			if (pass->get_type() == e_rgpass_type::graphics)
			{
				// allow uav writes?
				// ...

				// scope
				// ...

				graphics::renderpass_args args{};
				cmdlist->renderpass_begin(args);
				pass->execute();
				cmdlist->renderpass_end();
			}
		}
	}
#pragma endregion
}

