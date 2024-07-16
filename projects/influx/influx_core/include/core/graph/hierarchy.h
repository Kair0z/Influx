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
				: data{ m_data }, LayerIndex{ layer } {}

			uint32 LayerIndex;
			data data;
		};
		using node_vector = vector<node>;

	private:
		node m_root;
		vector<node_vector> m_layers{};

	public:
		hierarchy() = default;

		inline const node& add(const data& element, const node& parent)
		{
			const uint32 newLayerIndex = parent.LayerIndex + 1u;

			// Create new layer...
			if (newLayerIndex >= m_layers.dimension())
			{
				m_layers.push_back({});
			}
			
			m_layers[newLayerIndex].push_back({element, newLayerIndex});
			return *m_layers[newLayerIndex].end();
		}

		inline node* get_child(uint32 index, const node& parent)
		{
			if (!has_children(parent))
			{
				return nullptr;
			}

			return nullptr;
		}

		inline uint32 get_num_children(const node& parent) const
		{
			const uint32 childLayerIndex = parent.LayerIndex + 1u;
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