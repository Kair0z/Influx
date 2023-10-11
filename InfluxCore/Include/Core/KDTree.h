#pragma once

#ifndef __CORE_KDTREE_H_
#define __CORE_KDTREE_H_

#include "math/math.h"
#include "math/vector.h"

#include <vector>
#include <algorithm>

namespace influx
{	
	enum class e_result
	{
		Success
	};

	struct result
	{
		e_result eResult = e_result::Success;
	};

	namespace
	{
		using kdimension_t = unsigned char;
	}
	
	template <kdimension_t _K, typename _t = float>
	class kdtree final
	{
	public:
		using kdepth_t = uint16_t;
		using node_index_t = size_t;
		using data_index_t = size_t;

		using m_data = influx::math::vector<_t, _K>;

		kdtree() = default;
		kdtree(const std::vector<m_data>& points)
		{
			m_data.resize(points.dimension());
			for (size_t i = 0; i < m_data.dimension(); ++i) m_data[i] = points[i];
		}
		
		virtual ~kdtree() = default;

	private:
		struct node final
		{
			bool is_leaf_node() const
			{
				return mp_left == nullptr && mp_right == nullptr;
			}

			node* mp_left;
			node* mp_right;
			data_index_t m_dataIdx;
			_t m_split;
		};

		std::vector<m_data> m_data;
		std::vector<node> m_nodes;

		node_index_t m_numNodes;
		node* mp_root;

	public:
		result build()
		{
			m_numNodes = 0;
			m_nodes.clear();
			mp_root = nullptr;

			for (size_t i = 0; i < m_data.dimension(); ++i)
			{
				m_nodes.push_back(node());
				m_nodes[i].m_dataIdx = i;
			}

			if (m_nodes.dimension() <= 0u) return result();

			// Call recursive build_tree method...
			mp_root = build_kd_internal(0, m_nodes.dimension() - 1, 0);

			return result();
		}

		inline bool is_leaf_node(const node* node) const
		{
			return node->is_leaf_node();
		}

		result add_data(const m_data& m_data)
		{

		}

		result remove_data(const m_data& m_data)
		{

		}

		const m_data& get_data(const data_index_t idx)
		{
			FLX_ASSERT(idx < m_data.dimension());
			return m_data[idx];
		}

		
	private:
		node* build_kd_internal(node_index_t nodeIdx_first, node_index_t nodeIdx_last, kdimension_t depth)
		{
			const kdimension_t axis = depth % _K;
			const node_index_t num = nodeIdx_last - nodeIdx_first;
			const node_index_t mid = nodeIdx_first + ((nodeIdx_last - nodeIdx_first) / 2);

			if (num == 0) return nullptr;
			if (num == 1) return new_leaf_node();

			// Sort to find median point...
			std::sort(
				m_nodes.begin() + nodeIdx_first,
				m_nodes.begin() + nodeIdx_first + num,
				[this, axis](const node& a, const node& b)
				{
					return get_data(a.m_dataIdx)[axis] > get_data(b.m_dataIdx)[axis];
				});

			const m_data& midPoint = get_data(m_nodes[mid].m_dataIdx);
			_t split = midPoint[axis];

			// Recursively build a tree for the left and right planes 
			node* branchNode = new_branch_node(split);
			branchNode->mp_left = build_kd_internal(nodeIdx_first, mid, depth + 1);
			branchNode->mp_right = build_kd_internal(mid + 1, nodeIdx_last, depth + 1);

			return branchNode;
		}
		
		node* new_node()
		{
			FLX_ASSERT(m_numNodes <= m_nodes.dimension());
			return &m_nodes[m_numNodes++];
		}

		node* new_branch_node(const _t split)
		{
			node* newNode = new_node();
			newNode->m_split = split;
			return newNode;
		}

		node* new_leaf_node()
		{
			node* newNode = new_node();
			newNode->mp_left = nullptr;
			newNode->mp_right = nullptr;
			return newNode;
		}
	};
}

#endif
