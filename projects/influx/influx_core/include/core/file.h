#pragma once

#include "string.h"
#include "container/vector.h"
#include "debug.h"
#include <filesystem>
#include <functional>

#include "core/function.h"

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

	class file_watcher
	{
	public:
		typedef void (*on_change)(const file& file);
		typedef void (*on_rename)(const file& file);
		typedef void (*on_delete)(const file& file);
		typedef void (*on_create)(const file& file);

		enum notify_filters : uint8_t
		{
			filename	= 1 << 0,
			dirname		= 1 << 1,
			attributes	= 1 << 2,
			size		= 1 << 3,
			lastwrite	= 1 << 4,
			lastaccess	= 1 << 5,
			creation	= 1 << 6,
			security	= 1 << 7
		};

		file_watcher() = default;
		file_watcher(const file& target)
			: m_target{ target }
		{
			set_target(target);
		}

		void set_target(const file& file)
		{
			auto path = std::filesystem::path(file.m_path_full);
			m_target = file;
			m_last_write = std::filesystem::last_write_time(path);
		}

		const file& get_target() const
		{
			return m_target;
		}

		bool is_watching_valid_file()
		{
			return file::exists(m_target);
		}

		void check_file()
		{
			bool is_valid_now = is_watching_valid_file();
			if (is_valid_now)
			{
				auto path = m_target.m_path_full;
				auto status = std::filesystem::status(path);
				auto last_write = std::filesystem::last_write_time(path);

				string new_name = "";
				if (file::is_file_renamed(m_target, m_last_write, new_name))
				{
					call_on_rename();
				}

				if (m_last_write != last_write)
				{
					call_on_change();
					m_last_write = last_write;
				}

				if (m_was_valid == false)
				{
					// not very accurate
					// call_on_create();
				}

				return;
			}

			if (!is_valid_now && m_was_valid)
			{
				call_on_delete();
			}
		}

		void subscribe_onchange(on_change clb) { m_on_change_list.push_back(clb); }
		void subscribe_ondelete(on_delete clb) { m_on_delete_list.push_back(clb); }
		void subscribe_oncreate(on_create clb) { m_on_create_list.push_back(clb); }
		void subscribe_onrename(on_rename clb) { m_on_rename_list.push_back(clb); }

		void unsub_onchange(on_change clb) { m_on_change_list.remove(clb); }
		void unsub_ondelete(on_delete clb) { m_on_delete_list.remove(clb); }
		void unsub_oncreate(on_create clb) { m_on_create_list.remove(clb); }
		void unsub_onrename(on_rename clb) { m_on_rename_list.remove(clb); }

	private:
		notify_filters m_filters;
		bool m_include_subdirs = false;
		file m_target;
		bool m_was_valid;

		std::filesystem::file_time_type m_last_write;

		list<on_change> m_on_change_list{};
		list<on_rename> m_on_rename_list{};
		list<on_delete> m_on_delete_list{};
		list<on_create> m_on_create_list{};
	
		void call_on_change() { for (on_change clb : m_on_change_list) { clb(m_target); } }
		void call_on_rename() { for (on_rename clb : m_on_rename_list) { clb(m_target); } }
		void call_on_delete() { for (on_delete clb : m_on_delete_list) { clb(m_target); } }
		void call_on_create() { for (on_create clb : m_on_create_list) { clb(m_target); } }
	};
}