#pragma once

#if _DLL
	#define INFLUX_INPUT_API __declspec(dllexport)
#else
	#define INFLUX_INPUT_API __declspec(dllimport)
#endif

#include "core/platform/platform.h"
#include "core/platform/window.h"

namespace influx::input
{
	struct init_args final
	{
		platform::instance_handle m_instance;
		platform::window_handle m_window;
	};

	INFLUX_INPUT_API void init(const init_args& args = {});

	INFLUX_INPUT_API void cleanup();
}