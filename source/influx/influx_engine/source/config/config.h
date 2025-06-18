#pragma once

// influx::engine
#include "influx_engine.h"

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"

namespace influx::engine
{
	struct engine_config
	{
		path m_file_influx_root;
		path m_file_influx_assets;
		path m_file_influx_staged;
		path m_file_influx_resources;
	};

	struct app_config
	{
		using self = app_config;
		
		INFLUX_ENGINE_API
		self& set_window_dim(const math::vectoru2& dim);

		math::vectoru2 m_window_dimensions;
	};

	struct game_config final
	{
		using self = game_config;
		
		INFLUX_ENGINE_API
		self& set_gameproject_path(const string&);

		INFLUX_ENGINE_API
		self& set_name(const string& name);

		string m_gameproject_path;
		string m_gamename;
	};

	struct editor_config
	{

	};
}