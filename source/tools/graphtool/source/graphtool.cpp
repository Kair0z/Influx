#include "graphtool.h"

#if 0
#include "core/container/map.h"
#include "core/container/vectormap.h"

namespace influx::graphtool
{
	class node_data final
	{
	public:

	};

	class graph_data final
	{
	public:
		vectormap<node_id, node> m_nodemap;
	};

	node::node()
	{
		m_data = new node_data();
	}
	node::~node()
	{
		delete m_data;
	}

	graph::graph()
	{
		m_data = new graph_data();
	}
	graph::~graph()
	{
		delete m_data;
	}

	result<node*> graph::find_node(const node_id& id)
	{
		using result_type = result<node*>;
		return m_data->m_nodemap.find(id);
	}

	result<node const*> graph::find_node(const node_id& id) const
	{
		using result_type = result<node const*>;
		return m_data->m_nodemap.find(id);
	}

	result<graph> graph::parse(const string& filepath)
	{
		using result_type = result<graph>;

		return {};
	}
}
#endif

void graph_tool::tick(app::plugin::app_interface& app)
{
	app.m_commands.push();
}

#include "imgui.h"
void graph_tool::tick_imgui()
{
	ImGui::Begin("graph tool");

	ImGui::End();
}
