#pragma once

#if _DLL
	#define INFLUX_IMGUI_API __declspec(dllexport)
#else
	#define INFLUX_IMGUI_API __declspec(dllimport)
#endif

// imgui dependency (duh)
#include "imgui.h"

// influx::graphics dependency 
#include "influx_graphics.h"

namespace influx::imgui
{
	INFLUX_IMGUI_API bool initialize();

	INFLUX_IMGUI_API bool shutdown();

	INFLUX_IMGUI_API void render(ImDrawData* draw_data);
}