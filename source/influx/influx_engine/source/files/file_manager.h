#pragma once
#include "core/file.h"
#include "core/string.h"

namespace influx::engine
{
	enum class engine_directory : uint8
	{
		root,			/* /Influx/... */
		config,			/* /Influx/config/... */
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

	class file_manager final
	{
	public:
		file_manager();
		~file_manager();

		path get_absolute(engine_directory directory) const
		{
			// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
			const string exe_directory = engine::get_run_argument("exe_dir");
			const string& root = exe_directory + "/../../../";
			switch (directory)
			{
			case engine_directory::root:			return root;
			case engine_directory::config:			return root + "/config/";
			case engine_directory::assets:			return root + "/assets/engine/";
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
	};
}