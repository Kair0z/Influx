#pragma once
#include <vector>

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

		enum class e_traverse : uint8_t
		{
			breadth,
			depth,
			count
		};

		class node final
		{
			
		};

		inline _t get_child()
		{
			return m_data;
		}

	private:
		_t m_data{};
	};
}