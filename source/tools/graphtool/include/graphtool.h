#pragma once

#if _DLL
#define INFLUX_GRAPHTOOL_API __declspec(dllexport)
#else
#define INFLUX_GRAPHTOOL_API __declspec(dllimport)
#endif

#include "plugins/plugins.h"

using namespace influx;

class graph_tool final : public app::plugin
{
public:
	INFLUX_GRAPHTOOL_API virtual void tick(app::plugin::app_interface& app) override;

	INFLUX_GRAPHTOOL_API virtual void tick_imgui() override;
};

#if 0
#include "core/string.h"
#include "core/result.h"
#include "core/container/vector.h"
#include "core/math/vector.h"

namespace influx::graphtool
{
	using string = influx::string;
	class node_data;
	class graph_data;

	using node_id = uint64;
	static node_id make_id(const string& str)
	{
		hash<string> hasher;
		return hasher(str);
	}

	class node final
	{
		friend class graph;
		node(); ~node();
		node_data* m_data = nullptr;
	public:
		INFLUX_GRAPHTOOL_API void add_field();
		INFLUX_GRAPHTOOL_API void remove_field();
	};

	class graph final
	{
		graph_data* m_data = nullptr;
	public:
		INFLUX_GRAPHTOOL_API graph();
		INFLUX_GRAPHTOOL_API ~graph();
		INFLUX_GRAPHTOOL_API result<node*> find_node(const node_id& id);
		INFLUX_GRAPHTOOL_API result<node const*> find_node(const node_id& id) const;
		INFLUX_GRAPHTOOL_API static result<graph> parse(const string& filepath);
	};
}
#endif