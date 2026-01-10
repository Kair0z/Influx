#pragma once

// influx::core
#include "core/file.h"

// tom++
#include "toml.hpp"

namespace influx::engine
{
	class project final
	{
		path m_filepath;
		toml::v3::parse_result m_parsed_result{};
		bool m_is_valid = false;
		bool m_is_dirty = false;

		result<> recursive_write(toml::v3::table& table, const string& key, const string& value);

	public:
		project() = default;
		static result<project> load(const path& filepath);
		
		result<> write(const string& key, const string& value, bool write_to_file);
		
		// flushes changes to the result onto the file
		result<> flush();
		
		result<string> get_name() const;
		result<string> get_version() const;

		~project() = default;
	};
}