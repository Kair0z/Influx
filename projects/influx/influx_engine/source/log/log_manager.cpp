#include "engine_pch.h"
#include "log_manager.h"

// influx::engine
#include "file/engine_files.h"

// influx::core
#include "core/log.h"

namespace influx::engine
{
	static string g_filepath = "";

	log_manager::log_manager()
	{
		static string intermediate = get_engine_directory(engine_directory::intermediate).m_path_full;
		g_filepath = intermediate + "/log/engine.log";

		m_categories.resize(k_capacity);
		m_lines.resize(k_capacity);

		if (file::exists(g_filepath))
		{
			file::duplicate(g_filepath);
			file::clear(g_filepath);
		}
		else
		{
			file::create(g_filepath);
		}
	}

	log_manager::~log_manager()
	{
		flush_to_file();
	}

	void log_manager::tick()
	{
		flush_to_file();
	}

	void log_manager::flush_to_file()
	{
		influx_assert(file::push_lines(g_filepath, m_lines));
		m_counter = 0u;
	}
}