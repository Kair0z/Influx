#pragma once

// influx::core
#include "core/graph/hierarchy.h"

// influx::engine
#include "layer/layer.h"

namespace influx::engine
{
	class layergraph final
	{
		flat_tree<layer*> m_graph{};
		using graph_node = flat_tree<layer*>::node;

	public:
		template <class _ltype, class ..._args>
		_ltype* create_layer(layer* parent, _args&&... args)
		{
			_ltype* new_layer = new _ltype(args...);
			new_layer->m_owner = this;
			add_to_graph(new_layer, parent);
			return new_layer;
		}

		void destroy_layer(layer* layer)
		{
			if (layer != nullptr)
			{
				remove_from_graph(layer);

				delete layer;
				layer = nullptr;
			}
		}

		void update(const update_context& ctx);

		void on_keydown(input::e_key);
		void on_keyup(input::e_key);
		void on_ascii_down(char);
		void on_ascii_up(char);
		void on_mouse_move(const input::mouse_position& position);
		void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position);
		void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position);

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
				flat_tree<layer*>::node* found_parent_node = m_graph.find(parent);
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

		void remove_from_graph(layer* child)
		{
			flat_tree<layer*>::node* found_node = m_graph.find(child);
			if (found_node != nullptr)
			{
				m_graph.remove(*found_node);
			}
		}
	};
}