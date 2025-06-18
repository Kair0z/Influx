#pragma once

#include "string.h"
#include "container/vector.h"
#include "debug.h"
#include "core/function.h"
#include "core/result.h"

// STL
#include <fstream>
#include <string>
#include <filesystem>

namespace influx
{
	class path final
	{
		using char_type = wchar_t;
		using other_char_type = char;
		using str_type = wstring;
		using other_str_type = string;

		inline static str_type convert(const other_str_type& in)
		{
			return to_wstring(in);
		}

	public:
		template <typename _t = char>
		using result = influx::result<_t, const char*>;

	private:
		bool m_is_directory = false;
		bool m_is_file = false;
		bool m_exists = false;

		str_type m_full_path = L"";
		str_type m_filename = L"";
		str_type m_filename_without_extension = L"";
		str_type m_extension = L"";
		str_type m_directory = L"";

	private:
		inline void initialize(const str_type& str)
		{
			std::filesystem::path path(str);

			const wstring filename = path.filename().wstring();
			const wstring directory = path.parent_path().wstring() + L"/";
			const wstring extension = path.extension().wstring();

			m_filename	= filename.c_str();
			m_directory = directory.c_str();
			m_full_path = path.c_str();
			m_extension = extension.c_str();
			m_filename_without_extension = filename.substr(0u, filename.size() - extension.size()).c_str();
		}

	public:
		/* stored paths */
		inline const str_type& get_full_path() const
		{ return m_full_path; }

		inline const str_type& get_filename(bool without_extension) const
		{ return without_extension ? m_filename_without_extension : m_filename; }

		inline const str_type& get_extension() const
		{ return m_extension; }

		inline const str_type& get_directory() const
		{ return m_directory; }

		/* OS file query operations */
		inline static bool exists(const str_type& in_path)
		{
			std::filesystem::path path(in_path);
			return std::filesystem::exists(path);
		}

		inline static bool exists(const other_str_type& in_path)
		{
			return exists(convert(in_path));
		}

		inline bool exists() const
		{ 
			return exists(m_full_path); 
		}

		inline static bool is_directory(const str_type& in_path)
		{
			if (!is_valid_directory_path(in_path))
			{
				return false;
			}

			std::filesystem::path path(in_path);
			return std::filesystem::is_directory(path);
		}

		inline static bool is_directory(const other_str_type& in_path)
		{
			return is_directory(convert(in_path));
		}

		inline static bool is_valid_directory_path(const str_type& cstr)
		{
			// todo...
			return true;
		}

		inline static bool is_valid_filepath(const str_type& cstr)
		{
			// todo...
			return true;
		}

		inline bool is_directory() const
		{ 
			return is_directory(m_full_path);
		}
		
		inline static result<> create_directory(const str_type& cstr)
		{
			if (!is_valid_directory_path(cstr))
			{
				return result<>::make_error("cstr is not a valid directory path!");
			}

			std::filesystem::path path(cstr);
			return std::filesystem::create_directory(path);
			return {};
		}

		inline static result<> create_directory(const other_str_type& in_path)
		{
			return create_directory(convert(in_path));
		}

		inline static result<> create_file(const str_type& in_path)
		{
			if (!is_valid_filepath(in_path))
			{
				return result<>::make_error("in_path is not a valid file path!");
			}

			std::filesystem::path path(in_path);

			// create the directory chain
			std::filesystem::create_directories(path.parent_path());

			std::ofstream fstream(in_path);
			if (!fstream.is_open())
			{
				return result<>::make_error("failed opening write filestream to path!");
			}

			return {};
		}

		inline static result<> create_file(const other_str_type& in_path)
		{
			return create_file(convert(in_path));
		}

		inline static result<vector<path>> get_files_in_directory(const str_type& directory, bool recursive, const str_type& file_extension = {})
		{
			using result_type = result<vector<path>>;

			if (!is_directory(directory))
				return result_type::make_error("in_path is not valid!");

			vector<path> out_files{};
			auto push_file = [&out_files, file_extension](const std::filesystem::directory_entry& entry)
				{
					const str_type& full_path = entry.path().wstring();
					const str_type& filename = entry.path().filename().wstring();
					const str_type& extension = entry.path().extension().wstring();

					if (!extension.empty() && extension != file_extension)
					{
						// no matching extension!
						return;
					}

					auto extension_start = filename.find(extension);

					out_files.push_back(path{});
					path& the_file = out_files.back();
					the_file.m_full_path = full_path.c_str();
					the_file.m_extension = extension.c_str();
					the_file.m_filename = filename.substr(0u, extension_start).c_str();
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

		inline static result<vector<path>> get_files_in_directory(const other_str_type& path_str, bool recursive, const other_str_type& file_extension = {})
		{
			return get_files_in_directory(convert(path_str), recursive, convert(file_extension));
		}

		inline static result<> copy_file_contents(const str_type& src_path, const str_type& dest_path)
		{
			if (exists(src_path) == false)
				return result<>::make_error("src_path is not valid!");
			if (exists(dest_path) == false)
				return result<>::make_error("dest_path is not valid!");

			std::ifstream ifs(src_path);
			if (!ifs.is_open())
				return result<>::make_error("failed opening ifstream!");

			std::ofstream ofs(dest_path);
			if (!ofs.is_open())
				return result<>::make_error("failed opening ofstream!");

			string line = "";
			while (std::getline(ifs, line))
			{
				ofs << line << "\n";
			}
			return {};
		}

		inline static result<uint32> get_num_lines_in_file(const str_type& src_path)
		{
			using result_type = result<uint32>;
			std::ifstream file(src_path, std::ios::binary);
			if (!file.is_open())
			{
				return result_type::make_error("failed opening ifstream for path!");
			}

			return (uint32)std::count(std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>(), '\n');
		}

		inline static result<string> read_all_to_string(const str_type& path)
		{
			using result_type = result<string>;

			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (file)
			{
				string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				return content;
			}

			return result_type::make_error("failed opening file at path!");
		}

		inline static result<string> read_all_to_string(const other_str_type& path)
		{
			return read_all_to_string(convert(path));
		}

		// creates a duplicate file with number appended
		inline static result<> duplicate_file(const str_type& src_path, const str_type& dup_extension = nullptr)
		{
			if (!exists(src_path))
				return result<>::make_error("file at src_path doesn't exist!");

			path src_as_path{ src_path };
			const str_type& directory = src_as_path.m_directory;
			const str_type& filename = src_as_path.m_filename;

			size_t insert_point = filename.find_last_of('.');
			uint32 count = 0u; str_type new_name = filename;
			while (exists((directory + new_name).c_str()) && count < 1000)
			{
				uint64 found = new_name.find_last_of('_');
				if (found < new_name.size())
				{
					// if the path has a version tag,
					// parse the version tag and increment it
					const uint32 number = std::stoul(new_name.substr(found + 1u));
					new_name = new_name.substr(0u, found) + L"_" + to_wstring(number + 1u) + src_as_path.m_extension;
				}
				else
				{
					// if path doesnt have a version number yet...
					// "file" -> "file_0"
					new_name = new_name.insert(insert_point, L"_" + to_wstring(count++));
				}
			}

			const str_type new_path = directory + new_name;

			// create new file with new path, and copy old contents
			auto create_res = create_file(new_path.c_str());
			if (create_res.is_unex())
			{
				return result<>::make_error("failed creating a new file!");
			}

			auto copy_res = copy_file_contents(src_path, new_path.c_str());
			if (copy_res.is_unex())
			{
				return result<>::make_error("failed copying file contents!");
			}

			return {};
		}

		inline static result<> duplicate_file(const other_str_type& src_path, const str_type& dup_extension = nullptr)
		{
			return duplicate_file(convert(src_path), dup_extension);
		}

		// o(n): direct indexing is not supported :sad:
		inline static result<vector<string>> get_lines(const str_type& path, const uint32 start_index, const uint32 max_index = uint32(-1))
		{
			using result_type = result<vector<string>>;
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

		inline static result<vector<string>> get_lines(const other_str_type& path, const uint32 start_index, const uint32 max_index = uint32(-1))
		{
			return get_lines(convert(path), start_index, max_index);
		}

		inline static string content_to_string(const string& path)
		{
			std::ifstream file(path); // Open the file

			std::stringstream buffer;
			buffer << file.rdbuf(); // Read the entire file into the buffer
			return buffer.str();
		}

		inline static result<> clear_content(const str_type& path)
		{
			if (exists(path.c_str()))
			{
				std::ofstream file(path, std::ios::trunc); // Open in truncation mode
				file.close(); // Closing the file ensures changes are save
			}

			return result<>::make_error("path does not exist!");
		}

		inline static result<> clear_content(const other_str_type& path)
		{
			return clear_content(convert(path));
		}

		inline static bool push_line(const string& path, const string& line)
		{
			return push_lines(path, { line });
		}

		inline static bool push_lines(const string& path, const vector<string>& lines)
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
		inline static bool scoped_push_lines(const string& path, _func&& func)
		{
			std::ofstream file(path, std::ios::app);
			func(file);
			file.close();
			return true;
		}

		inline static bool is_text(const string& file)
		{
			std::filesystem::path path(file);
			return std::filesystem::is_regular_file(path);
		}

		inline static bool is_file_renamed(const path& file, const std::filesystem::file_time_type& last_time, str_type& out_new_name)
		{
			std::filesystem::path path(file.m_full_path);
			std::filesystem::directory_iterator dir_it(path.parent_path());

			for (const auto& entry : dir_it)
			{
				const bool is_file = !entry.is_directory();
				if (!is_file) continue;

				const str_type new_name = entry.path().filename().wstring();
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

		/* constructors */
		path() = default;

		inline path(const other_str_type& filepath)
		{
			initialize(convert(filepath));
		}

		inline path(const char_type* cstr)
		{
			initialize(str_type(cstr));
		}

		inline path(const other_char_type* cstr)
		{
			initialize(convert(other_str_type(cstr)));
		}

		inline path(const str_type& filepath)
		{
			initialize(filepath);
		}
	};
}