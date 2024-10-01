#pragma once

// influx::core
#include "../string.h"
#include "../container/map.h"
#include "../debug.h"

// influx::core::platform
#include "../platform/platform_thread.h"

// stl
#include <thread>

namespace influx
{
	class thread final
	{
	public:
		template <class _func, class... _args>
		explicit thread(const string& name, _func&& func, _args&&... args)
			: thread(std::forward<_func>(func), std::forward<_args>(args)...)
		{
			set_name(name);
		}

		template <class _func, class... _args>
		explicit thread(_func&& func, _args&&... args)
		{
			m_thread = std::thread(std::forward<_func>(func), std::forward<_args>(args)...);
			set_name("");
		}

		virtual ~thread()
		{
			if (m_thread.joinable())
				m_thread.join();
		}

		void set_name(const string& name)
		{
			g_id_to_name_map[m_thread.get_id()] = name;
			// platform::set_current_thread_name(name);
		}

		string& get_name() const
		{
			influx_assert(g_id_to_name_map.contains(m_thread.get_id()));
			return g_id_to_name_map[m_thread.get_id()];
		}

		void join()
		{
			m_thread.join();
		}

		void detach()
		{
			m_thread.detach();
		}

		void swap(thread& other)
		{
			m_thread.swap(other.m_thread);
		}

	private:
		std::thread m_thread;

		static umap<std::thread::id, string> g_id_to_name_map;
	};

	umap<std::thread::id, string> thread::g_id_to_name_map = {};
}