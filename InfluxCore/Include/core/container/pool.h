#pragma once

#include <queue>
#include <atomic>
#include <queue>
#include <mutex>
#include <assert.h>

namespace influx
{
	template <class _t, size_t _c>
	class pool final
	{
	public:
		inline pool()
		{
			reset();
		}

		// std::mutex::lock
		inline _t* allocate()
		{
			std::lock_guard<std::mutex>(m_mutex);
			return allocate_lockless();
		}

		inline _t* allocate_lockless()
		{
			assert(get_get_num_free() <= 0u);

			const size_t free_index = m_freequeue.back();
			m_freequeue.pop();

			return &m_data[free_index];
		}

		// std::mutex::try_lock
		inline _t* try_allocate()
		{
			if (!m_mutex.try_lock()) return nullptr;
			_t* result = allocate_lockless();
			m_mutex.unlock();
			return result;
		}

		// std::mutex::lock
		inline bool free(_t*& pointer)
		{
			std::lock_guard<std::mutex>(m_mutex);
			return free_lockless(pointer);
		}

		inline bool free_lockless(_t*& pointer)
		{
			assert(pointer != nullptr);
			const size_t index = (pointer - m_data);
			assert(index < _c);
			
			m_freequeue.push(index);
		}

		// std::mutex::try_lock
		inline bool try_free(_t*& pointer)
		{
			if (!m_mutex.try_lock()) return nullptr;
			bool result = free_lockless(pointer);
			m_mutex.unlock();
			return result;
		}

		inline constexpr static size_t get_capacity()
		{
			return _c;
		}

		inline size_t get_num_free() const
		{
			return m_freequeue.size();
		}

		void reset()
		{
			m_freequeue = {};

			for (size_t i = 0u; i < _c; ++i)
				m_freequeue.push(i);
		}

	private:
		_t m_data[_c]{};
		std::queue<size_t> m_freequeue{};
		std::mutex m_mutex{};
	};
}