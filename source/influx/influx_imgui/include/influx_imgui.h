#pragma once

#if _DLL
	#define INFLUX_IMGUI_API __declspec(dllexport)
#else
	#define INFLUX_IMGUI_API __declspec(dllimport)
#endif

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/imgui_widgets.h"

// influx::graphics
#include "influx_graphics.h"

// imgui
struct ImDrawData;

namespace influx::imgui
{
	INFLUX_IMGUI_API bool initialize();

	INFLUX_IMGUI_API bool shutdown();

	struct target final
	{
		graphics::render_target_view* mp_rtv;
	};

	INFLUX_IMGUI_API void render(ImDrawData* draw_data, const target& target);
}