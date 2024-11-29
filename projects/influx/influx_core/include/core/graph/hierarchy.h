#pragma once

#include "core/container/containers.h"
#include "core/function.h"

namespace influx
{
	template <class _t>
	class hierarchy final
	{
	public:
		using data = _t;
		constexpr static uint32 k_maxLayers = u32_max;
		constexpr static uint32 k_maxChildren = u32_max;

		struct node final
		{
			node() = default;
			node(const data& m_data, uint32 layer)
				: data{ m_data }, m_layer_idx{ layer } {}

			uint32 m_layer_idx{};
			data data{};
		};
		using node_vector = vector<node>;

	private:
		node m_root{};
		vector<node_vector> m_layers{};

	public:
		hierarchy() = default;

		inline node* find_node(const data& value)
		{
			node* found = nullptr;
			traverse([&found, &value](node& node)
			{
				if (node.data == value)
				{
					found = &node;
				}
			});

			return found;
		}

		inline const node& add(const data& element, const node& parent)
		{
			const uint32 new_layer_index = parent.m_layer_idx + 1u;

			while (new_layer_index >= m_layers.size())
			{
				m_layers.push_back({});
			}
			
			node_vector& this_layer = m_layers[new_layer_index];
			this_layer.push_back({element, new_layer_index });
			const node& new_node = this_layer.back();
			return new_node;
		}

		inline void remove(node& nod)
		{
			const uint32 layer_idx = nod.m_layer_idx;
			if (layer_idx < m_layers.size())
			{
				node_vector& layer = m_layers[layer_idx];
				layer.pop_back();
			}
		}

		inline node* get_child(uint32 index, const node& parent)
		{
			if (!has_children(parent))
			{
				return nullptr;
			}

			const uint32 child_layer_idx = parent.m_layer_idx + 1u;
			node_vector& child_layer = m_layers[child_layer_idx];
			if (index < child_layer.size())
			{
				return &child_layer[index];
			}
			else
			{
				return nullptr;
			}
		}

		inline uint32 get_num_children(const node& parent) const
		{
			const uint32 childLayerIndex = parent.m_layer_idx + 1u;
			if (childLayerIndex >= m_layers.size())
			{
				return 0u;
			}

			return 1u;
		}

		inline bool has_children(const node& parent) const
		{
			return get_num_children(parent) != 0u;
		}

		inline void traverse(const function<void(node&)>& traverse_func)
		{
			function<void(node&)> visit_children;
			visit_children = [this, &visit_children, &traverse_func](node& parent)
			{
				for (uint32 i = 0u; i < get_num_children(parent); ++i)
				{
					node* child = get_child(i, parent);
					if (child)
					{
						traverse_func(*child);
					}
				}

				for (uint32 i = 0u; i < get_num_children(parent); ++i)
				{
					node* child = get_child(i, parent);
					if (child)
					{
						visit_children(*child);
					}
				}
			};

			visit_children(m_root);
		}

		inline node& get_root()
		{
			return m_root;
		}
	};
}