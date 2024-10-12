#include "engine_pch.h"

namespace influx::engine
{
	void game_module::on_config(config& config)
	{
		config
			.set_gamefile("")
			.set_window_dim({ 640u, 480u });
	}

	void game_module::on_start()
	{
		
	}

	void game_module::on_update()
	{
		
	}

	void game_module::on_cleanup()
	{
	}

	game_module::config::self& game_module::config::set_gamefile(const string& file)
	{
		m_gamefile_path = file;
		return *this;
	}

	game_module::config::self& game_module::config::set_window_dim(const math::vectoru2& dim)
	{
		m_window_dimensions = dim;
		return *this;
	}

	game_module::config::self& game_module::config::set_name(const string& name)
	{
		m_gamename = name;
		return *this;
	}
}