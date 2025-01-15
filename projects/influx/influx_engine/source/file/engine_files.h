#pragma once

// influx::core
#include "core/file.h"
#include "core/string.h"

// influx::platform
#include "influx_platform/platform.h"

namespace influx::engine
{
	enum class engine_directory : uint8
	{
		root,
		assets,
		staged,
		intermediate,
		binaries,
		games,
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

	static file get_engine_directory(engine_directory directory)
	{
		// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
		const string& root = platform::platform::get_current_directory() + "/../../../";
		switch (directory)
		{
			case engine_directory::root:			return root;
			case engine_directory::assets:		return root + "/assets/";
			case engine_directory::staged:		return root + "/staged/";
			case engine_directory::binaries:		return root + "/bin/";
			case engine_directory::intermediate: return root + "/int/";
			case engine_directory::games:		return root + "/games/";
			case engine_directory::shaderpdb:	return root + "/int/shaderdebug/";
		}
		return {};
	}

	static file get_game_directory(const string& game_name, game_directory directory)
	{
		const file& games_directory = get_engine_directory(engine_directory::games);
		const file game_directory = games_directory.m_path_full + "/" + game_name + "/";
		influx_assert(game_directory.is_directory());
		switch (directory)
		{
			case game_directory::root: return game_directory;
			case game_directory::assets: return game_directory.m_path_full + "/assets/";
			case game_directory::binaries: return game_directory.m_path_full + "/binaries/";
		}
		return {};
	}
}