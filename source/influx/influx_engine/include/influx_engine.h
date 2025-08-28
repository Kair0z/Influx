#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

// =============================================================================
// Engine application frontend
// =============================================================================
namespace influx::engine
{
	INFLUX_ENGINE_API void run_editor(int argc = 0, char* argv[] = nullptr);

	INFLUX_ENGINE_API void run_game(int argc = 0, char* argv[] = nullptr);
}
