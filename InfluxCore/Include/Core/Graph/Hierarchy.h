#pragma once

#ifndef __CORE_GRAPH_HIERARCHY_H_
#define __CORE_GRAPH_HIERARCHY_H_

#include "core/container/containers.h"

namespace influx
{
	template <class _t>
	class hierarchy final
	{
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

	public:
		hierarchy() = default;

		const node& add(const data& element, const node& parent)
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

		const node* get_child(uint32 index, const node& parent)
		{
			if (!has_children())
			{
				return nullptr;
			}
		}

		uint32 get_num_children(const node& parent) const
		{
			const uint32 childLayerIndex = parent.LayerIndex + 1u;
			if (childLayerIndex >= m_layers.dimension())
			{
				return 0u;
			}

			return 1u;
		}

		bool has_children(const node& parent) const
		{
			return get_num_children() != 0u;
		}

	private:
		node m_root;

		vector<node_vector> m_layers{};
	};
}

#endif