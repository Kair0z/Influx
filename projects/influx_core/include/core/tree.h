#pragma once
#include <vector>

#include <set>
namespace influx
{
	class tree_view
	{

	};

	template <typename _t>
	class ntree final
	{
	public:
		enum class e_node_flags : uint8_t
		{
			unused,
			used,
			// ...
			count
		};

		enum class e_traverse_order : uint8_t
		{
			breadth_first,
			depth_first,
			count
		};

		class node final
		{
			std::set<int> ints{};
		};

		tree()
		{

		}

		node& get_root()
		{

		}

		_t get_child()
		{

		}

	private:
		_t m_data{};
	};
}