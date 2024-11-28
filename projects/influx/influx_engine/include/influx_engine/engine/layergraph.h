#pragma once

// influx::core
#include "core/graph/hierarchy.h"

// influx::engine
#include "influx_engine/engine/layer.h"

namespace influx::engine
{
	class layergraph final
	{
		hierarchy<layer*> m_graph{};
		using graph_node = hierarchy<layer*>::node;

	public:
		template <class _ltype, class ..._args>
		_ltype* create_layer(layer* parent, _args&&... args)
		{
			_ltype* new_layer = new _ltype(args...);
			new_layer->m_owner = this;
			add_to_graph(new_layer, parent);
			return new_layer;
		}

		void update(const update_context& ctx)
		{
			m_graph.traverse([&ctx](const graph_node& node)
			{
				layer* layer = node.data;
				if (layer == nullptr) return;

				if (layer->m_frame_counter == 0u)
				{
					layer->on_start();
				}
				else
				{
					layer->on_update(ctx);
				}

				layer->m_frame_counter++;
			});
		}

	private:
		void add_to_graph(layer* child, layer* parent = nullptr)
		{
			if (parent == nullptr)
			{
				// add to root
				m_graph.add(child, m_graph.get_root());
			}
			else
			{
				hierarchy<layer*>::node* found_parent_node = m_graph.find_node(parent);
				if (found_parent_node != nullptr)
				{
					m_graph.add(child, *found_parent_node);
				}
				else
				{
					influx_assert(false);
				}
			}
		}
	};
}