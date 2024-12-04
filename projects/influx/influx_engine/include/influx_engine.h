#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

namespace influx::engine
{
	INFLUX_ENGINE_API void run_editor();

	INFLUX_ENGINE_API void run_game();
}
