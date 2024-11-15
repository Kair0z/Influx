#pragma once

#include "string.h"
#include "container/vector.h"
#include "debug.h"
#include <filesystem>
#include <functional>

namespace influx
{
	struct directory
	{
		string m_path_full;
	};

	struct file
	{
		file() = default;
		file(const string& filepath)
		{
			std::filesystem::path path(filepath);

			m_filename = path.filename().string();
			m_path_full = path.string();
			m_extension = path.extension().string();
		}

		static bool is_directory(const string& string)
		{
			std::filesystem::path path(string);
			return std::filesystem::is_directory(path);
		}

		bool is_directory() const
		{
			return is_directory(m_path_full);
		}

		static bool make_directory(const string& string)
		{
			std::filesystem::path path(string);
			return std::filesystem::create_directory(path);
		}

		static bool create(const string& file)
		{
			std::filesystem::path path(file);
			std::filesystem::create_directories(path.parent_path());
			return true;
		}

		static bool exists(const file& file)
		{
			return std::filesystem::exists(file.m_path_full);
		}

		static bool exists(const string& file)
		{
			std::filesystem::path path(file);
			return std::filesystem::exists(path);
		}

		string m_path_full;
		string m_filename;
		string m_extension;
	};

	inline bool create_file(const string& file)
	{
		return file::create(file);
	}

	inline vector<file> get_files_in_directory(const string& directory, bool recursive, const string& file_extension = {})
	{
		influx_assert(std::filesystem::is_directory(directory));
		
		vector<file> out_files{};
		auto push_file = [&out_files, file_extension](const std::filesystem::directory_entry& entry)
		{
			const string& full_path = entry.path().string();
			const string& filename = entry.path().filename().string();
			const string& extension = entry.path().extension().string();

			if (!extension.empty() && extension != file_extension)
			{
				// no matching extension!
				return;
			}

			auto extension_start = filename.find(extension);

			out_files.push_back(file{});
			file& the_file = out_files.back();
			the_file.m_path_full = full_path;
			the_file.m_extension = extension;
			the_file.m_filename = filename.substr(0u, extension_start);
		};

		if (recursive)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
			{
				if (entry.is_regular_file())
				{
					push_file(entry);
				}
			}
		}
		else
		{
			for (const auto& entry : std::filesystem::directory_iterator(directory))
			{
				push_file(entry);
			}
		}
		
		return out_files;
	}
}