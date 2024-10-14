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

	void game_module::on_level_loaded()
	{
	}

	void game_module::on_update(const ctx_update& ctx)
	{
	}

	void game_module::on_cleanup()
	{
	}

	void game_module::load_level(level* level)
	{
	}

	void game_module::load_level(const string& levelname)
	{
	}

	level const* game_module::get_current_level() const
	{
		return nullptr;
	}

	const game_module::config& game_module::get_config() const
	{
		return m_config;
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

	game_module::ctx_update::ctx_update(const frame_time& frtime)
		: m_frametime{ frtime }
	{

	}
}