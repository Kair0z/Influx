#pragma once

// influx::core
#include "core/file.h"
#include "core/string.h"

// influx::platform
#include "influx_platform/platform.h"

// influx::engine
#include "engine.h"

namespace influx::engine
{
	enum class engine_directory : uint8
	{
		root,
		assets,
		assets_gen,
		staged,
		intermediate,
		binaries,
		games,
		editor,
		shaderpdb,
		count
	};

	enum class game_directory : uint8
	{
		root,
		assets,
		binaries,
		count
	};

	inline static path get_engine_directory(engine_directory directory)
	{
		// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
		const string exe_directory = engine::get_run_argument("exe_dir");
		const string& root = exe_directory + "/../../../";
		switch (directory)
		{
			case engine_directory::root:			return root;
			case engine_directory::assets:			return root + "/assets/";
			case engine_directory::assets_gen:		return root + "/assets/generated/";
			case engine_directory::staged:			return root + "/staged/";
			case engine_directory::binaries:		return root + "/bin/";
			case engine_directory::intermediate:	return root + "/int/";
			case engine_directory::games:			return root + "/games/";
			case engine_directory::editor:			return root + "/editor/";
			case engine_directory::shaderpdb:		return root + "/int/shaderdebug/";
		}
		return {};
	}

	inline static path get_game_directory(const string& game_name, game_directory directory)
	{
		const path& games_directory = get_engine_directory(engine_directory::games);
		const string games_dir_path_str = to_string(games_directory.get_full_path());
		const path game_directory = games_dir_path_str + "/" + game_name + "/";
		influx_assert(game_directory.is_directory());
		switch (directory)
		{
			case game_directory::root:		return game_directory;
			case game_directory::assets:	return games_dir_path_str + "/assets/";
			case game_directory::binaries:	return games_dir_path_str + "/binaries/";
		}
		return {};
	}

	inline static string get_friendly_name(const string& path)
	{
		// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
		const string exe_directory = engine::get_run_argument("exe_dir");
		const string& root = exe_directory + "/../../../";

		if (str::contains(path, root))
		{
			return path.substr(root.size());
		}

		return path;
	}
}