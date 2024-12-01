#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

#pragma region dependencies
// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"
#include "core/result.h"

// influx::engine
#include "influx_engine/game/game.h"
#include "influx_engine/editor/editor.h"
#include "influx_engine/engine/config.h"
#include "influx_engine/engine/common.h"
#include "influx_engine/engine/component.h"
#include "influx_engine/engine/gameobject.h"
#include "influx_engine/engine/layergraph.h"
#pragma endregion

namespace influx::engine
{
	INFLUX_ENGINE_API void run_engine();

	INFLUX_ENGINE_API void run_editor();
}

