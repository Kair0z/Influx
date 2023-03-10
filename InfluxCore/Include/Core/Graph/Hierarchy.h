#pragma once

#ifndef __CORE_GRAPH_HIERARCHY_H_
#define __CORE_GRAPH_HIERARCHY_H_

#include "Core/Container/Containers.h"

namespace Influx
{
	template <class _T>
	class Hierarchy final
	{
		using Data = _T;

		constexpr static uint32 k_maxLayers = u32_max;
		constexpr static uint32 k_maxChildren = u32_max;

		struct Node final
		{
			Node() = default;
			Node(const Data& data, uint32 layer)
				: Data{ data }, LayerIndex{ layer } {}

			uint32 LayerIndex;
			Data Data;
		};

		using NodeList = Vector<Node>;

	public:
		Hierarchy() = default;

		const Node& Add(const Data& element, const Node& parent)
		{
			const uint32 newLayerIndex = parent.LayerIndex + 1u;

			// Create new layer...
			if (newLayerIndex >= m_layers.size())
			{
				m_layers.push_back({});
			}
			
			m_layers[newLayerIndex].push_back({element, newLayerIndex});
			return *m_layers[newLayerIndex].end();
		}

		const Node* GetChild(uint32 index, const Node& parent)
		{
			if (!HasChildren())
			{
				return nullptr;
			}
		}

		uint32 GetNumChildren(const Node& parent) const
		{
			const uint32 childLayerIndex = parent.LayerIndex + 1u;
			if (childLayerIndex >= m_layers.size())
			{
				return 0u;
			}

			return 1u;
		}

		bool HasChildren(const Node& parent) const
		{
			return GetNumChildren() != 0u;
		}

	private:
		Node m_root;

		Vector<NodeList> m_layers{};
	};
}

#endif