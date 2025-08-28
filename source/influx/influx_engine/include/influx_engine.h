#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

#include "core/result.h"

// =============================================================================
// Engine application frontend
// =============================================================================
namespace influx::engine
{
	template <typename _t = char>
	using result = result<_t, const char*>;

	INFLUX_ENGINE_API result<> run_editor(int argc = 0, char* argv[] = nullptr);

	INFLUX_ENGINE_API result<> run_game(int argc = 0, char* argv[] = nullptr);
}
