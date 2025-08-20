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
		root,			/* /Influx/... */
		assets,			/* /Influx/assets/... */
		assets_gen,		/* /Influx/assets/generated/... */
		staged,			/* /Influx/staged/... */
		intermediate,	/* /Influx/int/... */
		binaries,		/* /Influx/bin/... */
		projects,		/* /Influx/influx_projects/... */
		editor,			/* /Influx/editor/... */
		shaderpdb,		/* /Influx/int/shaderdebug/... */
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
			case engine_directory::projects:		return root + "/influx_projects/";
			case engine_directory::editor:			return root + "/editor/";
			case engine_directory::shaderpdb:		return root + "/int/shaderdebug/";
		}
		return {};
	}

	inline static path get_project_directory(const string& game_name, game_directory directory)
	{
		const path& projects_directory = get_engine_directory(engine_directory::projects);
		const string projects_dir_path = to_string(projects_directory.get_full_path());
		const path game_directory = projects_dir_path + "/" + game_name + "/";
		influx_assert(game_directory.is_directory());
		switch (directory)
		{
			case game_directory::root:		return game_directory;
			case game_directory::assets:	return projects_dir_path + "/assets/";
			case game_directory::binaries:	return projects_dir_path + "/binaries/";
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