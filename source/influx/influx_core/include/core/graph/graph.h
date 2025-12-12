#pragma once

#ifndef _CORE_GRAPH_H_
#define _CORE_GRAPH_H_

#include <ostream>
#include <string>
#include <utility>

namespace influx
{
#ifndef __CORE_STRING_H_
	using string = std::string;
#endif

	using uint32 = unsigned int;
	using upair = std::pair<uint32, uint32>;

	// this class keeps connection info between raw indices
	// 
	template <uint32 _c = 30u, bool k_digraph = true>
	class graph final
	{
		// a graph of 1 is useless
		static_assert(_c > 1);

	public:
		using node_id = uint32;
		// 0:- | 1:-> | 2:<- | 3:<->
		enum linkstate : uint32
		{
			DISABLED = 0,
			FORW	 = 1 << 0,
			BACK	 = 1 << 1,
			TWOWAY   = FORW | BACK
		};

	private:
		static linkstate get_mirror_state(linkstate state)
		{
			switch (state)
			{
			case FORW: return BACK;
			case BACK: return FORW;
			}
			return state;
		}
		static constexpr uint32 k_max_nodes = _c;
		static constexpr uint32 k_max_links = (_c * (_c - 1)) / 2;
		static constexpr uint32 k_max_links_per_node = _c - 1;
		static constexpr const char* linkstate_cstring[4]
		{
			" ",
			"->",
			"<-",
			"<->"
		};

		static constexpr uint32 k_invalid = (uint32)-1;
		struct link final
		{
			linkstate m_state;
		};
		link m_links[k_max_links];
		uint32 m_num_active_links = 0;

	public:
		static uint32 get_link_index(node_id a, node_id b)
		{
			// triangular number formula
			if (b < a) std::swap(a, b);
			return a * (2 * k_max_nodes - a - 1) / 2 + (b - a - 1);
		}

		static upair get_node_ids(const uint32 link_index)
		{
			// Calculate which row i this edge belongs to
			int i = 0;
			int edges_in_prev_rows = 0;

			// This looks like a loop but can be converted to closed-form
			while (i < k_max_nodes - 1) {
				int edges_in_current_row = k_max_nodes - i - 1;
				if (link_index < edges_in_prev_rows + edges_in_current_row) {
					break;
				}
				edges_in_prev_rows += edges_in_current_row;
				i++;
			}

			// Calculate j
			int j = i + 1 + (link_index - edges_in_prev_rows);

			return { i, j };
		}

		void set_link(node_id a, node_id b, linkstate new_state)
		{
			if (b == a)
				return;

			// invert the operation if B < A
			// this is necessary, because when we GET item handles for a given index,
			// the handles will always be returned from smallest index to biggest index.
			// iow, getting the state of a link at index ALWAYS reflects the perspective 
			// of the smallest index node of the pair
			if (b < a)
				new_state = get_mirror_state(new_state);

			const uint32 link_index = get_link_index(a, b);
			link& link = m_links[link_index];
			const linkstate old_state = link.m_state;

			// ensure digraph correctness!
			if (new_state == graph::TWOWAY && k_digraph)
			{
				assert(false);
				return;
			}
			
			link.m_state = new_state;
			if (old_state != DISABLED && new_state == DISABLED) m_num_active_links--;
			else if (old_state == DISABLED && new_state != DISABLED) m_num_active_links++;
		}

		// similar to set_link(), but oldstate |= newstate
		// keep in mind if new_state == DISABLED(0), this is a noop
		void add_link(node_id a, node_id b, linkstate new_state)
		{
			const uint32 link_index = get_link_index(a, b);

			// adjust the final state
			const linkstate old_state = m_links[link_index].m_state;
			const linkstate merged_state = linkstate(old_state | new_state);
			new_state = merged_state;
			set_link(a, b, new_state);
		}

		// useful for when the order of the objects changes.
		// at the end, a will have b's connections and vice versa
		void swap_links(node_id a, node_id b)
		{
			if (a == b)
				return;
			if (b > a)
				std::swap(a, b);

			// swap all link states of nodes connected to either a or b
			for (uint32 i = 0u; i < k_max_nodes; ++i)
			{
				if (i == a || i == b)
					continue;

				uint32 i0 = get_link_index(a, i);
				uint32 i1 = get_link_index(b, i);
				std::swap(m_links[i0], m_links[i1]);

				const bool node_attached_to_a = m_links[i0].m_state != DISABLED;
				const bool node_attached_to_b = m_links[i1].m_state != DISABLED;
				if (node_attached_to_a && node_attached_to_b)
				{
					m_links[i0].m_state = get_mirror_state(m_links[i0].m_state);
					m_links[i1].m_state = get_mirror_state(m_links[i1].m_state);
				}
			}

			// if linked to eachother, flip the link
			link& interlink = m_links[get_link_index(a, b)];
			if (interlink.m_state != DISABLED)
				interlink.m_state = get_mirror_state(interlink.m_state);
		}

		// remove all links to/from item
		void detach(node_id node)
		{
			for (uint32 i = 0u; i < k_max_nodes; ++i)
			{
				if (node == i) continue;
				set_link(node, i, DISABLED);
			}
		}

		uint32 find_num_loops(node_id start = 0u) const
		{
			if (is_empty())
				return 0u;

			if (m_num_active_links < 2)
			{

			}
		}

		void clear()
		{
			for (uint32 i = 0u; i < k_max_links; ++i)
			{
				m_links[i].m_state = DISABLED;
			}
		}

		bool is_empty() const
		{
			return m_num_active_links == 0u;
		}

		uint32 get_num_nodes() const
		{
			return (m_num_active_links * 2) + 1;
		}

		const link& get_link(const uint32 index) const
		{
			return m_links[index];
		}

		/// <summary>
		/// dotgraph exporter:
		/// </summary>
		typedef void (*dotgraph_decorator)(const node_id&, string&);
		static void default_dotgraph_decorator(const node_id& node, string& out_label)
		{
			out_label = std::to_string(node);
		}

		template <typename _decorator>
		static void to_dotgraph(const graph& graph, std::ostream& os, _decorator&& decorator = default_dotgraph_decorator)
		{
			os << "\ndigraph g {\n";
			os << "compound=true;\n";
			os << "ranksep=0.4;\n";

#if 0
			if constexpr (k_ignore_loners == false)
			{
				for (uint32 i = 0u; i < graph.get_num_items(); ++i)
				{
					const item& item = graph.get_item(i);
					os << std::to_string(i).c_str() << " ";

					string item_label = "";
					decorator(item, item_label);

					os << "[shape=box, label=\"" << item_label << "\"];";
					os << "\n";
				}
			}
#endif

			for (uint32 i = 0u; i < k_max_links; ++i)
			{
				const linkstate state = graph.get_link(i).m_state;
				if (state == DISABLED)
					continue;

				upair nodes = get_node_ids(i);
				if (state == BACK)
					std::swap(nodes.first, nodes.second);
				
				os << "\n";
				string label;
				decorator(nodes.first, label);
				os << "\"" << (std::string)label << "\"";
				os << " -> ";
				// os << " " << linkstate_cstring[state] << " ";
				decorator(nodes.second, label);
				os << "\"" << (std::string)label << "\"";
				os << " ";

				// node descriptor
				os << " [color=\"#000000\"];";
			}
			os << "\n}";
		}
	};

	template <typename _t, uint32 _n>
	class graph_array final
	{
		std::array<_t, _n> m_data;
		using graph = graph<_n, true>;
		graph m_graph;
		uint32 m_num = 0u;

	public:
		void add(const _t& value)
		{
			if (m_num >= _n)
				return;

			const uint32 index = m_num;
			m_data[index] = value;

			if (index > 1)
				m_graph.set_link(m_num - 2, m_num - 1, graph::FORW);
			m_num++;
		}

		void remove(const _t& value)
		{
			auto found = std::find(m_data.cbegin(), m_data.cend(), value);
			if (found >= m_data.cend())
				return;

			const uint32 index = std::distance(m_data.cbegin(), found);
			if (index == m_num - 1)
			{
				m_graph.detach(index);
				m_num--;
				return;
			}
			else
			{
				// swap & pop trick
				std::swap(m_data[index], m_data[m_num - 1]);
				m_graph.detach(m_num - 1);
				m_num--;
			}
		}

		void to_dotgraph(std::ostream& os)
		{
			graph::to_dotgraph(m_graph, os, [this](const graph::node_id& node_id, string& out_label)
			{
				out_label = m_data[node_id];
			});
		}
	};
}

#endif