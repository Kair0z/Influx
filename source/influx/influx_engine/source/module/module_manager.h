#pragma once

// influx::core
#include "core/container/map.h"
#include "core/string.h"

namespace influx::platform
{
	class library;
}

namespace influx::engine
{
	enum class e_module_state
	{
		unloaded,
		loaded
	};

	struct module_signature final
	{
		string m_name;
		string m_filepath;

		bool operator==(const module_signature& other) const
		{
			return m_name == other.m_name && m_filepath == other.m_filepath;
		}
	};
}
namespace std
{
	template<>
	struct hash<influx::engine::module_signature>
	{
		std::size_t operator()(const influx::engine::module_signature& sig) const
		{
			return std::hash<string>()(sig.m_name);
		}
	};
}

namespace influx::engine
{
	class module final
	{
	public:
		string m_name = "";
		e_module_state m_state;
		platform::library* m_library = nullptr;
		uint32 m_num_times_loaded = 0u;

		void call_function(const string& function_name) const;
	};

	class module_manager final
	{
	public:
		module_manager();
		~module_manager();

		void update();

		result<module> find_module(const string& filepath);
		result<module> find_module_with_name(const string& name) const;

		result<> call_module_function(const string& module_name, const string& func_name) const;

	private:
		umap<module_signature, module> m_modules{};
	};
}