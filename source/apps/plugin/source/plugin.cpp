#include "plugin.h"

// influx::engine
#include <iostream>
#include "influx_engine/engine_api.h"

influx::plugin_api* plugin::m_engine = nullptr;
void plugin::engine_init(influx::plugin_api* api)
{
	m_engine = api;
}