#pragma once

#ifndef _CORE_KDTREE_H_
#define _CORE_KDTREE_H_

#include "Math/Math.h"
#include "Math/Vector.h"

#include <vector>
#include <algorithm>

namespace Influx
{	
	enum class EResult
	{
		Success
	};

	struct Result
	{
		EResult eResult = EResult::Success;
	};

	namespace
	{
		using KDimension_t = unsigned char;
	}
	
	template <KDimension_t _K, typename _T = float>
	class KDTree final
	{
	public:
		using KDepth_t = uint16_t;
		using NodeIndex_t = size_t;
		using DataIndex_t = size_t;

		using Data = Influx::Math::Vector<_T, _K>;

		KDTree() = default;
		KDTree(const std::vector<Data>& points)
		{
			m_data.resize(points.size());
			for (size_t i = 0; i < m_data.size(); ++i) m_data[i] = points[i];
		}
		
		virtual ~KDTree() = default;

	private:
		struct Node final
		{
			bool IsLeafNode() const
			{
				return mp_left == nullptr && mp_right == nullptr;
			}

			Node* mp_left;
			Node* mp_right;
			DataIndex_t m_dataIdx;
			_T m_split;
		};

		std::vector<Data> m_data;
		std::vector<Node> m_nodes;

		NodeIndex_t m_numNodes;
		Node* mp_root;

	public:
		Result Build()
		{
			m_numNodes = 0;
			m_nodes.clear();
			mp_root = nullptr;

			for (size_t i = 0; i < m_data.size(); ++i)
			{
				m_nodes.push_back(Node());
				m_nodes[i].m_dataIdx = i;
			}

			if (m_nodes.size() <= 0u) return Result();

			// Call recursive build_tree method...
			mp_root = Internal_BuildKD(0, m_nodes.size() - 1, 0);

			return Result();
		}

		inline bool IsLeafNode(const Node* node) const
		{
			return node->IsLeafNode();
		}

		Result AddDataPoint(const Data& data)
		{

		}

		Result RemoveDataPoint(const Data& data)
		{

		}

		const Data& GetData(const DataIndex_t idx)
		{
			FLX_ASSERT(idx < m_data.size());
			return m_data[idx];
		}

		
	private:
		Node* Internal_BuildKD(NodeIndex_t nodeIdx_first, NodeIndex_t nodeIdx_last, KDimension_t depth)
		{
			const KDimension_t axis = depth % _K;
			const NodeIndex_t num = nodeIdx_last - nodeIdx_first;
			const NodeIndex_t mid = nodeIdx_first + ((nodeIdx_last - nodeIdx_first) / 2);

			if (num == 0) return nullptr;
			if (num == 1) return GetNewLeafNode();

			// Sort to find median point...
			std::sort(
				m_nodes.begin() + nodeIdx_first,
				m_nodes.begin() + nodeIdx_first + num,
				[this, axis](const Node& a, const Node& b)
				{
					return GetData(a.m_dataIdx)[axis] > GetData(b.m_dataIdx)[axis];
				});

			const Data& midPoint = GetData(m_nodes[mid].m_dataIdx);
			_T split = midPoint[axis];

			// Recursively build a tree for the left and right planes 
			Node* branchNode = GetNewBranchNode(split);
			branchNode->mp_left = Internal_BuildKD(nodeIdx_first, mid, depth + 1);
			branchNode->mp_right = Internal_BuildKD(mid + 1, nodeIdx_last, depth + 1);

			return branchNode;
		}
		
		Node* GetNewNode()
		{
			FLX_ASSERT(m_numNodes <= m_nodes.size());
			return &m_nodes[m_numNodes++];
		}

		Node* GetNewBranchNode(const _T split)
		{
			Node* newNode = GetNewNode();
			newNode->m_split = split;
			return newNode;
		}

		Node* GetNewLeafNode()
		{
			Node* newNode = GetNewNode();
			newNode->mp_left = nullptr;
			newNode->mp_right = nullptr;
			return newNode;
		}
	};
}

#endif
