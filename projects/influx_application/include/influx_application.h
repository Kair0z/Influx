#pragma once

#if _DLL
	#define INFLUX_APP_API __declspec(dllexport)
#else
	#define INFLUX_APP_API __declspec(dllimport)
#endif

// core dependencies
#include "core/basetypes.h"
#include "core/string.h"
#include "core/math/vector.h"

namespace influx::application
{
	struct run_args final
	{
		inline run_args(int argc = 0, char** argv = nullptr)
			: m_argv{argv}
			, m_argc{argc}
		{
		}

		const char* m_name = "";
		string m_resources_dir = "";

		bool m_commandlet = false;
		bool m_enable_scenerender = false;
		bool m_enable_editor = false;
		bool m_enable_game = false;
		bool m_vsync = false;
		bool m_single_threaded = false;

		math::vectorf4 m_window_clear_colour = {};

		uint32 m_window_width = 640u;
		uint32 m_window_height = 480u;

		int m_argc{};
		char** m_argv{};
	};

	void INFLUX_APP_API run(const run_args& args);
	void INFLUX_APP_API quit();
}