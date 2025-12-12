#include "engine_pch.h"
#include "project.h"

// STL
#include <fstream>
#include <iostream>
#include <sstream>

namespace influx::engine
{
	using namespace toml::v3;

	result<project> project::load(const path& filepath)
	{
		using result_type = result<project>;

		if (filepath.exists() == false)
			return result_type::make_error("project filepath doesn't exist!");

		project out_project = {};
		const std_str& filepath_str = filepath.get_full_path().get_std();
		out_project.m_parsed_result = toml::parse_file(filepath_str);
		out_project.m_is_valid = true;
		out_project.m_filepath = filepath;
		return out_project;
	}

	result<> project::recursive_write(toml::v3::table& table, const string& key, const string& value)
	{
		result<> res{};
		table.for_each([&](auto& lkey, auto& lvalue)
		{
			if constexpr (toml::is_table<decltype(lvalue)>)
			{
				res = recursive_write(lvalue, key, value);
			}
			else if constexpr (toml::is_string<decltype(lvalue)>)
			{
				if (key.get_std() == lkey)
				{
					lvalue = value.get_std();
					return;
				}
				else res = result<>::make_error("no key found!");
			}
		});
		return res;
	}

	result<> project::write(const string& key, const string& value, bool write_to_file)
	{
		if (!m_is_valid)
			return result<>::make_error("project is not loaded valid!");

		if (!recursive_write(m_parsed_result, key, value))
			return result<>::make_error("failed writing to key!");

		m_is_dirty = true;

		if (write_to_file) flush();
		return {};
	}

	result<> project::flush()
	{
		if (!m_is_valid)
			return result<>::make_error("project is not loaded valid!");

		if (!m_is_dirty) return {}; // not an error
		else
		{
			std::stringstream ss{};
			ss << m_parsed_result;

			path::overwrite(m_filepath.get_full_path(), ss.str());
		}
		return {};
	}

	result<string> project::get_name() const
	{
		if (!m_is_valid)
			return result<string>::make_error("project is not loaded valid!");

		const node* node = m_parsed_result.get("name");
		if (node->is_string())
		{
			return string(node->as_string()->get());
		}
		else return result<string>::make_error("value at key:version is not a string!");
	}

	result<string> project::get_version() const
	{
		if (!m_is_valid)
			return result<string>::make_error("project is not loaded valid!");

		const node* node = m_parsed_result.get("version");
		if (node->is_string())
		{
			return string(node->as_string()->get());
		}
		else return result<string>::make_error("value at key:version is not a string!");
	}
}