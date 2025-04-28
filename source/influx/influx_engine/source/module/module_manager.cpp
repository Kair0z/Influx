#include "engine_pch.h"
#include "module_manager.h"

// influx::platform
#include "influx_platform/library.h"

namespace influx::engine
{
	result<module> module_manager::find_module(const string& filepath)
	{
		using result_type = result<module>;

		const bool is_dll_path = filepath.ends_with(".dll");
		if (!is_dll_path)
			return result_type::make_error("error: path doesn't point to a dll!");

		file as_file{ filepath };
		const string filenam = as_file.m_filename_without_extension;

		// load the dll module
		platform::library* lib = platform::library::load(filepath);
		if (lib == nullptr)
			return result_type::make_error("error: failed loading .dll library at path!");

		// free old if exists
		module_signature signature{ .m_name = filenam, .m_filepath = filepath };
		if (m_modules.contains(signature))
		{
			platform::library* old_lib = m_modules[signature].m_library;
			if (old_lib != nullptr)
			{
				platform::library::free(old_lib);
			}
		}

		module& mod = m_modules[signature];
		mod.m_library = lib;
		mod.m_state = e_module_state::loaded;
		mod.m_num_times_loaded++;

		return mod;
	}

	module_manager::module_manager()
	{
		
	}

	module_manager::~module_manager()
	{
		
	}

	void module_manager::update()
	{
		influx::engine::module game_mod = find_module("D:/Git/Influx/bin/debug-windows-x86_64/influx_game/influx_game.dll").get();
		if (game_mod.m_state == e_module_state::loaded)
		{
			for (const string& func_name : game_mod.m_library->get_functions())
			{
				game_mod.m_library->call(func_name);
			}
		}
	}

	
}