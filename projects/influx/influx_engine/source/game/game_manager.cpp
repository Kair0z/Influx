#include "engine_pch.h"

// influx::engine
#include "game_manager.h"
#include "engine.h"
#include "editor/editor_manager.h"

// influx::platform
#include "influx_platform/library.h"

namespace influx::engine
{
	inline platform::library* get_game_library()
	{
		// call game module dll start
		string dll_dir = "E:/Git/Influx/bin/debug-windows-x86_64/influx_game/";
		string dll_path = dll_dir + "influx_game.dll";

		// load dll
		static platform::library* lib = nullptr;
		if (lib == nullptr)
		{
			lib = platform::library::load(dll_path);
		}
		return lib;
	}

	void game_manager::start()
	{
		get_game_library()->call("start");
	}

	void game_manager::tick()
	{
		get_game_library()->call("tick");
	}

	void game_manager::end()
	{
		get_game_library()->call("end");
	}
}
