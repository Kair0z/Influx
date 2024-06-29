#pragma once

#ifndef _CORE_GRAPH_H_
#define _CORE_GRAPH_H_

#include <set>
#include <map>

// https://github.com/grame-cncm/digraph/blob/master
//===========================================================
// digraph : a directed graph, a set of nodes f type N and a
// set of connections between these nodes. Connections have an
// associated value, by defaut 0. This value is used in Faust
// to represent the time dependency between computations.
//===========================================================

namespace Influx
{
	namespace Internal
	{
		template <typename _N, typename _A>
		class Graph
		{
			using ConnectionMap = std::map<_N, _A>;

			std::set<_N>					m_nodes; // {n1,n2,...}
			std::map<_N, ConnectionMap>		m_connections; // {(ni -d-> nj),...}

		public:
			void AddNode(_N node)
			{
				m_nodes.insert(node);
				(void)m_connections[node]; // make sure we have an empty set of connections for n
			}

			void AddConnected(const _N& n1, const _N& n2, const _A& d)
			{
				AddNode(n1);
				AddNode(n2);

				auto& adj = m_connections[n1];
				auto existingConnection = adj.find(n2);

				if (existingConnection != adj.cend())
				{
					_A& d1 = existingConnection->second;
					d1 = (d1 < d) ? d1 : d; // Temp...
				}
				else
				{
					adj[n2] = d;
				}
			}

			// returns the set of nodes of the graph
			const std::set<_N>& GetNodes() const 
			{ 
				return m_nodes; 
			}

			// returns the connections of node n in the graph
			const std::map<_N, _A>& GetConnections(const _N& n) const 
			{ 
				return m_connections.at(n); 
			}

			// tests if two nodes are connected
			bool AreConnected(const _N& n1, const _N& n2, _A& d) const
			{
				auto c = m_connections.at(n1);
				auto q = c.find(n2);

				if (q != c.cend()) 
				{
					d = q->second;
					return true;
				}
				else 
				{
					return false;
				}
			}

			// tests if two nodes are connected
			bool AreConnected(const _N& n1, const _N& n2) const
			{
				_A d{};
				return AreConnected(n1, n2, d);
			}
		};
	}

	template <typename _N, typename _A>
	class DiGraph
	{
		Internal::Graph<_N, _A> m_internalGraph{};

	public:
		DiGraph() = default;

		DiGraph& AddNode(_N node)
		{
			m_internalGraph.AddNode(node);
			return *this;
		}

		// Add a graph with all its connections
		DiGraph& Add(const DiGraph& graph)
		{
			for (auto& n : graph.GetNodes())
			{
				AddNode(n);
				for (auto& c : graph.GetConnections(n)) 
				{
					AddConnected(n, c.first, c.second);
				}
			}

			return *this;
		}

		DiGraph& AddConnected(const _N& n1, const _N& n2, const _A& d)
		{
			m_internalGraph.AddConnected(n1, n2, d);
			return *this;
		}

		bool AreConnected(const _N& n1, const _N& n2, _A& d) const 
		{ 
			return m_internalGraph.AreConnected(n1, n2, d); 
		}

		bool AreConnected(const _N& n1, const _N& n2) const
		{
			return m_internalGraph.AreConnected(n1, n2);
		}

		const std::set<_N>& GetNodes() const 
		{ 
			return m_internalGraph.GetNodes(); 
		}

		const std::map<_N, _A>& GetConnections(const _N& n) const 
		{ 
			return m_internalGraph.GetConnections(n);
		}

		// compare graphs for maps and other containers
		friend bool operator<(const DiGraph& p1, const DiGraph& p2) { return p1.m_internalGraph < p2.m_internalGraph; }
		friend bool operator==(const DiGraph& p1, const DiGraph& p2) { return p1.m_internalGraph == p2.m_internalGraph; }

	private:
	};
}

#endif