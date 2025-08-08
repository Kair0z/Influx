#pragma once

#if _DLL
#define INFLUX_GAME_API __declspec(dllexport)
#else
#define INFLUX_GAME_API __declspec(dllimport)
#endif

namespace influx { struct engine_api; }

class INFLUX_GAME_API pong final
{
	static influx::engine_api* m_engine;

public:
	static void engine_init(influx::engine_api* api);
	static void start();
	static void tick();
	static void end();
};