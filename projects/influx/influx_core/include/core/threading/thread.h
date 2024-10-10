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
		thread() = default;
		
		template <class _func, class... _args>
		explicit thread(_func&& func, _args&&... args)
			: m_thread{ std::forward<_func>(func), std::forward<_args>(args)... }
		{
		}

		// std::thread moveable
		thread(thread&& other) noexcept
		{
			m_thread = std::move(other.m_thread);
		}

		thread& operator=(thread&& other) noexcept
		{
			m_thread = std::move(other.m_thread);
			return *this;
		}

		// std::thread non-copyable
		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;

		virtual ~thread()
		{
			if (m_thread.joinable())
				m_thread.join();
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
	};
}