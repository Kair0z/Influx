#pragma once

#include "string.h"
#include "container/vector.h"
#include "debug.h"
#include "core/function.h"

// STL
#include <fstream>
#include <string>
#include <filesystem>

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
			m_directory = path.parent_path().string() + "/";
			m_path_full = path.string();
			m_extension = path.extension().string();
			m_filename_without_extension = m_filename.substr(0u, m_filename.size() - m_extension.size());
		}

		file(const char* path)
			: file(string(path))
		{

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

			// create the directory chain
			std::filesystem::create_directories(path.parent_path());

			std::ofstream open_file(file);
			return open_file.is_open();
		}

		static bool exists(const string& file)
		{
			std::filesystem::path path(file);
			return std::filesystem::exists(path);
		}

		static bool copy_file(const string& source, const string& dest)
		{
			std::ifstream ifs(source);
			std::ofstream ofs(dest);
			if (!ifs.is_open() || !ofs.is_open())
			{
				return false;
			}

			string line = "";
			while (std::getline(ifs, line))
			{
				ofs << line << "\n";
			}
			return true;
		}

		// creates a duplicate file with number appended
		static bool duplicate(const string& path)
		{
			if (!exists(path)) return false;

			const file to_file = file(path);
			const string& directory = to_file.m_directory;
			const string& filename = to_file.m_filename;
			
			size_t insert_point = filename.find_last_of('.');
			uint32 count = 0u; string new_name = filename;
			while (exists(directory + new_name) && count < 1000)
			{
				uint64 found = new_name.find_last_of('_');
				if (found < new_name.size())
				{
					// if the path has a version tag,
					// parse the version tag and increment it
					const uint32 number = std::stoul(new_name.substr(found + 1u));
					new_name = new_name.substr(0u, found) + "_" + to_string(number + 1u) + to_file.m_extension;
				}
				else
				{
					// if path doesnt have a version number yet...
					// "file" -> "file_0"
					new_name = new_name.insert(insert_point, "_" + to_string(count++));
				}
			}

			const string new_path = directory + new_name;

			// create new file with new path, and copy old contents
			create(new_path);
			copy_file(path, new_path);

			return false;
		}

		static uint32 get_num_lines(const string& path)
		{
			std::ifstream file(path, std::ios::binary);
			if (!file) 
			{
				return 0u;
			}

			return (uint32)std::count(std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>(), '\n');
		}

		// o(n): direct indexing is not supported :sad:
		static vector<string> get_lines(const string& path, const uint32 start_index, const uint32 max_index = uint32(-1))
		{
			std::ifstream file(path);
			if (!file) { return {}; }

			vector<string> result_lines{};
			uint32 index = 0u;
			string line = "";
			while (index < max_index && std::getline(file, line))
			{
				if (index >= start_index)
				{
					result_lines.push_back(line);
				}
				index++;
			}
			return result_lines;
		}

		static string content_to_string(const string& path)
		{
			std::ifstream file(path); // Open the file

			std::stringstream buffer;
			buffer << file.rdbuf(); // Read the entire file into the buffer
			return buffer.str();
		}

		static bool clear(const string& path)
		{
			if (exists(path))
			{
				std::ofstream file(path, std::ios::trunc); // Open in truncation mode
				file.close(); // Closing the file ensures changes are save
			}
			return false;
		}

		static bool push_line(const string& path, const string& line)
		{
			return push_lines(path, { line });
		}

		static bool push_lines(const string& path, const vector<string>& lines)
		{
			std::ofstream file(path, std::ios::app);
			for (const string& str : lines)
			{
				file << str << "\n";
			}
			file.close();
			return true;
		}

		template <typename _func>
		static bool scoped_push_lines(const string& path, _func&& func)
		{
			std::ofstream file(path, std::ios::app);
			func(file);
			file.close();
			return true;
		}

		static bool is_text(const string& file)
		{
			std::filesystem::path path(file);
			return std::filesystem::is_regular_file(path);
		}

		static bool is_file_renamed(const file& file, const std::filesystem::file_time_type& last_time, string& out_new_name)
		{
			std::filesystem::path path(file.m_path_full);
			std::filesystem::directory_iterator dir_it(path.parent_path());
			
			for (const auto& entry : dir_it)
			{
				const bool is_file = !entry.is_directory();
				if (!is_file) continue;

				const string new_name = entry.path().filename().string();
				const bool is_name_different = new_name != file.m_filename;
				const bool is_same_write_time = entry.last_write_time() == last_time;
				if (entry.is_regular_file() 
					&& is_name_different 
					&& is_same_write_time)
				{
					out_new_name = new_name;
					return true;
				}
			}

			return false;
		}

		string m_path_full;
		string m_filename;
		string m_filename_without_extension;
		string m_extension;
		string m_directory;
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

	struct textfile
	{
		inline static string read_all(const string& path)
		{
			influx_assert(file::is_text(path));

			std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
			if (file) 
			{
				string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				return content;
			}

			return "";
		}
	};
}