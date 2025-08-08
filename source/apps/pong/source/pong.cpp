#include "pong.h"

// influx::engine
#include <iostream>
#include "influx_engine/engine_api.h"

influx::engine_api* pong::m_engine = nullptr;
void pong::engine_init(influx::engine_api* api)
{
	m_engine = api;
}

void pong::start()
{
	m_engine->log("pong::start");
}

void pong::tick()
{
	// m_engine->log("game::nick");
}

void pong::end()
{
	m_engine->log("pong::end");
}