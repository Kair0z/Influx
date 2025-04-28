#include "engine_pch.h"
#include "module_manager.h"

// influx::platform
#include "influx_platform/library.h"

namespace influx::engine
{
	result<module> module_manager::find_module(const string& filepath)
	{
		using result_type = result<module>;

		const bool file_exists = file::exists(filepath);
		if (!file_exists)
			return result_type::make_error("error: file doesn't exist!");

		const bool is_dll_path = filepath.ends_with(".dll");
		if (!is_dll_path)
			return result_type::make_error("error: path doesn't point to a dll!");

		file as_file{ filepath };
		const string filename = as_file.m_filename_without_extension;

		// load the dll module
		platform::library* lib = platform::library::load(filepath);
		if (lib == nullptr)
			return result_type::make_error("error: failed loading .dll library at path!");

		// free old if exists
		module_signature signature{ .m_name = filename, .m_filepath = filepath };
		if (m_modules.contains(signature))
		{
			platform::library* old_lib = m_modules[signature].m_library;
			if (old_lib != nullptr)
			{
				platform::library::free(old_lib);
			}
		}

		// update mod data
		module& mod = m_modules[signature];
		mod.m_library = lib;
		mod.m_state = e_module_state::loaded;
		mod.m_num_times_loaded++;
		mod.m_name = filename;

		return mod;
	}

	module_manager::module_manager()
	{
		// up-front find some modules
		find_module("D:/Git/Influx/bin/debug-windows-x86_64/influx_game/influx_game.dll");
	}

	module_manager::~module_manager()
	{

	}

	result<module> module_manager::find_module_with_name(const string& name) const
	{
		auto found = std::find_if(m_modules.cbegin(), m_modules.cend(), [&name](const auto& pair)
		{
			return pair.first.m_name == name;
		});

		// return the module if we found one
		if (found != m_modules.cend())
		{
			return (*found).second;
		}
		else return result<module>::make_error("error: module not found!");
	}

	result<> module_manager::call_module_function(const string& module_name, const string& func_name) const
	{
		if (module_name.empty())
			return result<>::make_error("error: empty module name!");

		if (func_name.empty())
			return result<>::make_error("error: empty function name!");

		auto res = find_module_with_name(module_name);
		if (res.is_unex())
			return result<>::make_error("error: module not found!");

		res.get().call_function(func_name);

		return {};
	}

	void module_manager::update()
	{
		influx::engine::module game_mod = find_module("D:/Git/Influx/bin/debug-windows-x86_64/influx_game/influx_game.dll").get();
		auto res = find_module_with_name("influx_game");
		if (res.is_success())
		{
			game_mod = res.get();
		}
	}

	void module::call_function(const string& function_name) const
	{
		if (m_library != nullptr)
		{
			m_library->call(function_name);
		}
	}
}