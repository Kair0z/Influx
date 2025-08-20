#pragma once

#if _DLL
#define INFLUX_PLUGIN_API __declspec(dllexport)
#else
#define INFLUX_PLUGIN_API __declspec(dllimport)
#endif

namespace influx { class plugin_api; }
class plugin final
{
	static influx::plugin_api* m_engine;

public:
	INFLUX_PLUGIN_API static void engine_init(influx::plugin_api* api);
};