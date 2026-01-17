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
		/* I hate strings >:( */
		using char_type = wchar_t;
		using other_char_type = char;
		using str_type = influx::string;

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
		uint64 m_content_hash = 0u;

	private:
		inline void initialize(const str_type& str)
		{
			std::filesystem::path path( str );
			const string extension = path.extension().wstring();

			m_filename = path.filename().wstring();
			m_directory = path.parent_path().wstring() + L"/";
			m_full_path = path.wstring();
			m_extension = path.extension().wstring();
			m_filename_without_extension = m_filename.substr(0u, m_filename.size() - extension.size());
		}

	public:
		/* stored paths */
		inline const str_type& get_full_path() const
		{ return m_full_path; }

		inline str_type get_full_path_without_extension() const {
			return m_directory + m_filename_without_extension;
		}

		inline const str_type& get_filename(bool without_extension) const
		{ return without_extension ? m_filename_without_extension : m_filename; }

		inline const str_type& get_extension() const
		{ return m_extension; }

		inline const str_type& get_directory() const
		{ return m_directory; }

		void append(const string& str)
		{
			(*this) = path(m_full_path + str);
		}

		path& operator+=(const string& str)
		{
			append(str); return *this;
		}

		inline void replace_extension(const char_type* str)
		{
			m_extension = str;
			m_filename = m_filename.substr(0u, m_filename.find('.')) + str;
			m_full_path = m_full_path.substr(0u, m_full_path.find('.')) + str;
		}

		inline result<uint64> get_content_hash(bool query)
		{
			if (query) query_content_hash();
			return m_content_hash;
		}

		inline result<uint64> query_content_hash()
		{
			using result_type = result<uint64>;

			if (!exists(m_full_path))
				return result_type::make_error("m_full_path is not a valid file!");

			auto content = content_to_string(m_full_path);
			if (!content)
				return result_type::make_error("content_to_string() failed!");

			std::hash<string> hasher;
			m_content_hash = hasher(content.get());
			return m_content_hash;
		}

		/* OS file query operations */
		inline static bool exists(const str_type& in_path)
		{
			std::filesystem::path path(in_path);
			return std::filesystem::exists(path);
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

		// todo...
		inline static bool is_valid_directory_path(const str_type& in_path)
		{
			return true;
		}

		// todo...
		inline static bool is_valid_filepath(const str_type& cstr)
		{
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

		inline static result<> create_file(const str_type& in_path, bool binary = false)
		{
			if (!is_valid_filepath(in_path))
			{
				return result<>::make_error("in_path is not a valid file path!");
			}

			std::filesystem::path path(in_path);

			// create the directory chain
			std::filesystem::create_directories(path.parent_path());

			if (binary)
			{
				std::wofstream fstream(in_path.get_wstd(), std::ios::binary);
				if (!fstream.is_open())
					return result<>::make_error("failed opening write filestream to path!");
			}
			else
			{
				std::wofstream fstream(in_path.get_wstd());
				if (!fstream.is_open())
					return result<>::make_error("failed opening write filestream to path!");
			}
			return {};
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

					out_files.push_back(path{ full_path });
				};

			if (recursive)
			{
				for (const auto& entry : std::filesystem::recursive_directory_iterator(directory.get_wstd()))
				{
					if (entry.is_regular_file())
					{
						push_file(entry);
					}
				}
			}
			else
			{
				for (const auto& entry : std::filesystem::directory_iterator(directory.get_wstd()))
				{
					push_file(entry);
				}
			}

			return out_files;
		}

		inline static result<> copy_file_contents(const str_type& src_path, const str_type& dest_path)
		{
			if (exists(src_path) == false)
				return result<>::make_error("src_path is not valid!");
			if (exists(dest_path) == false)
				return result<>::make_error("dest_path is not valid!");

			std::wifstream ifs(src_path.get_wstd());
			if (!ifs.is_open())
				return result<>::make_error("failed opening wifstream!");

			std::wofstream ofs(dest_path.get_wstd());
			if (!ofs.is_open())
				return result<>::make_error("failed opening ofstream!");

			string line = "";
			while (std::getline(ifs, line.get_wstd()))
			{
				ofs << line.get_wstd() << "\n";
			}
			return {};
		}

		inline static result<uint32> get_num_lines_in_file(const str_type& src_path)
		{
			using result_type = result<uint32>;
			std::wifstream file(src_path.get_wstd(), std::ios::binary);
			if (!file.is_open())
			{
				return result_type::make_error("failed opening ifstream for path!");
			}

			return (uint32)std::count(std::istreambuf_iterator<wchar_t>(file),
				std::istreambuf_iterator<wchar_t>(), '\n');
		}

		inline static result<string> read_all_to_string(const str_type& path)
		{
			using result_type = result<string>;
			std::wifstream file(path.c_wstr(), std::ios::in | std::ios::binary);
			if (file)
			{
				std_wstr wcontent{ std::istreambuf_iterator<wchar_t>(file), std::istreambuf_iterator<wchar_t>() };
				file.close();
				return string(wcontent);
			}
			return result_type::make_error("failed opening file at path!");
		}

		// creates a duplicate file with appended number
		inline static result<> duplicate_file(const str_type& src_path, const str_type& dup_extension = {})
		{
			if (!exists(src_path))
				return result<>::make_error("file at src_path doesn't exist!");

			path src_as_path{ src_path };
			const str_type& directory = src_as_path.m_directory;
			const str_type& filename = src_as_path.m_filename;

			size_t insert_point = filename.find_last_of('.');
			uint32 count = 0u; str_type new_name = filename;
			while (exists(directory + new_name) && count < 1000)
			{
				uint64 found = new_name.find_last_of('_');
				if (found < new_name.size())
				{
					// if the path has a version tag,
					// parse the version tag and increment it
					const uint32 number = std::stoul(new_name.substr(found + 1u).c_wstr());
					new_name = new_name.substr(0u, found) + L"_" + to_wstring(number + 1u) + src_as_path.m_extension;
				}
				else
				{
					// if path doesnt have a version number yet...
					// "file" -> "file_0"
					new_name.append( "_" + string(count++) );
				}
			}

			const str_type new_path = directory + new_name;

			// create new file with new path, and copy old contents
			auto create_res = create_file(new_path);
			if (create_res.is_fail())
			{
				return result<>::make_error("failed creating a new file!");
			}

			auto copy_res = copy_file_contents(src_path, new_path);
			if (copy_res.is_fail())
			{
				return result<>::make_error("failed copying file contents!");
			}

			return {};
		}

		// o(n): direct indexing is not supported :sad:
		inline static result<vector<string>> get_lines(const str_type& path, const uint32 start_index, const uint32 max_index = uint32(-1))
		{
			using result_type = result<vector<string>>;
			std::wifstream file(path.c_wstr());
			if (!file) { return {}; }

			vector<string> result_lines{};
			uint32 index = 0u;
			string line = "";
			while (index < max_index && std::getline(file, line.get_wstd()))
			{
				if (index >= start_index)
				{
					result_lines.push_back(line);
				}
				index++;
			}
			return result_lines;
		}

		inline static result<string> content_to_string(const str_type& path)
		{
			std::wifstream file(path.c_wstr()); // Open the file

			std::wstringstream buffer;
			buffer << file.rdbuf(); // Read the entire file into the buffer
			return string(buffer.str());
		}

		inline static result<> overwrite(const str_type& path, const string& new_content)
		{
			if (exists(path) == false)
			{
				auto created = create_file(path);
				if (!created)
					return result<>::make_error("failed creating new file!");
			}
			
			if (!clear_content(path))
				return result<>::make_error("failed clearing file at path!");
			if (!push_line(path, new_content))
				return result<>::make_error("failed pushing lines to path!");

			return {};
		}
		
		inline static result<> clear_content(const str_type& path)
		{
			if (exists(path))
			{
				std::wofstream file(path.c_wstr(), std::ios::trunc); // Open in truncation mode
				file.close(); // Closing the file ensures changes are save
				return {};
			}

			return result<>::make_error("path does not exist!");
		}

		inline static result<> push_line(const str_type& path, const string& line)
		{
			return push_lines(path, { line });
		}

		inline static result<> push_lines(const str_type& in_path, const vector<string>& lines)
		{
			std::wofstream fstream(in_path.get_wstd(), std::ios::app);
			if (fstream.is_open() == false)
				return result<>::make_error("failed opening ofstream to path!");

			for (const string& str : lines)
			{
				fstream << str.get_wstd() << L"\n";
			}
			fstream.close();
			return {};
		}

		template <typename _func>
		inline static result<> scoped_push_lines(const str_type& in_path, _func&& func)
		{
			std::ofstream file(in_path, std::ios::app);

			if (file.is_open() == false)
				return result<>::make_error("failed opening fstream to in_path.");

			func(file);
			file.close();
			return {};
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
		inline path(const char_type* cstr)
		{
			initialize(str_type(cstr));
		}
		inline path(const str_type& filepath)
		{
			initialize(filepath);
		}
		inline path(const path& other)
		{
			initialize(other.m_full_path);
		}
	};

	static path operator+(const path& lhs, const string& rhs)
	{
		return path(lhs.get_full_path() + rhs);
	}
}