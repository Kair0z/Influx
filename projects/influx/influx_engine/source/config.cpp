#include "engine_pch.h"

namespace influx::engine
{
	const game_config& game_module::get_config() const
	{
		return m_config;
	}

	// app config
	app_config::self& app_config::set_window_dim(const math::vectoru2& dim)
	{
		m_window_dimensions = dim;
		return *this;
	}

	// game config
	game_config::self& game_config::set_gameproject_path(const string& file)
	{
		m_gameproject_path = file;
		return *this;
	}

	game_config::self& game_config::set_name(const string& name)
	{
		m_gamename = name;
		return *this;
	}
}