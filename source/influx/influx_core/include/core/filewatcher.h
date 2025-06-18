#pragma once

// STL
#include <filesystem>

#define INFLUX_FILEWATCHER_STANDALONE 1
#if !INFLUX_FILEWATCHER_STANDALONE
#include "core/file.h"
#else
#include <string>
#include <list>
namespace influx
{
	template <typename _t>
	using list = std::list<_t>;
	using string = std::string;
	struct path
	{
		string m_path_full;
		string m_filename;

		path() = default;
		path(const string& filepath)
		{
			std::filesystem::path path(filepath);
			m_filename = path.filename().string();
			m_path_full = path.string();
		}

		static bool exists(const path& file)
		{
			return std::filesystem::exists(file.m_path_full);
		}

		static bool is_file_renamed(const path& file, const std::filesystem::file_time_type& last_time, string& out_new_name)
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

		void foo()
		{
			
		}
	};
}
#endif

namespace influx
{
	class file_watcher final
	{
	public:
		typedef void (*on_change)(const path&);
		typedef void (*on_rename)(const path&);
		typedef void (*on_delete)(const path&);
		typedef void (*on_create)(const path&);

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

		file_watcher(const path& target)
			: m_target{ target }
		{
			set_target(target);
		}

		void set_target(const path& file)
		{
			auto path = std::filesystem::path(file.m_path_full);
			m_target = file;
			m_last_write = std::filesystem::last_write_time(path);
		}

		const path& get_target() const
		{
			return m_target;
		}

		bool is_watching_valid_file()
		{
			return path::exists(m_target);
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
				if (path::is_file_renamed(m_target, m_last_write, new_name))
				{
					dispatch<on_rename>();
				}

				if (m_last_write != last_write)
				{
					dispatch<on_change>();
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
				dispatch<on_delete>();
			}
		}

		template <class _ev>
		void subscribe(_ev clb)
		{
			get_callback_list<_ev>().push_back(clb);
		}

		template <class _ev>
		void unsub(_ev clb)
		{
			get_callback_list<_ev>().remove(clb);
		}

	private:
		// unused
		notify_filters m_filters;
		
		// unused
		bool m_include_subdirs = false;

		//
		path m_target;

		// last check was valid
		bool m_was_valid;

		// last write timestamp we observed
		std::filesystem::file_time_type m_last_write;

		list<on_change> m_on_change_list{};
		list<on_rename> m_on_rename_list{};
		list<on_delete> m_on_delete_list{};
		list<on_create> m_on_create_list{};

		template <class _ev>
		list<_ev>& get_callback_list()
		{
			if constexpr (std::is_same_v<_ev, on_change>) {
				return m_on_change_list;
			}
			else if constexpr (std::is_same_v<_ev, on_rename>) {
				return m_on_rename_list;
			}
			else if constexpr (std::is_same_v<_ev, on_delete>) {
				return m_on_delete_list;
			}
			else if constexpr (std::is_same_v<_ev, on_create>) {
				return m_on_create_list;
			}
			else
			{
				// SHOULD NEVER BE THE CASE!
				return m_on_change_list;
			}
		}
	
		template <class _ev>
		void dispatch()
		{
			for (_ev clb : get_callback_list<_ev>())
			{
				clb(m_target);
			}
		}
	};
}