#pragma once

#include "core/debug.h"

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

		inline _t* allocate_lockless()
		{
			assert(get_num_free() > 0u);

			const size_t free_index = m_freelist.back();
			m_freelist.pop_back();

			return &m_data[free_index];
		}
		inline _t* allocate()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return allocate_lockless();
		}
		inline std::vector<_t*> allocate_lockless(size_t num)
		{
			std::vector<_t*> result{};
			for (size_t i = 0u; i < num; ++i)
			{
				_t* alloc = allocate_lockless();
				influx_assert_not_null(alloc);
				result.push_back(alloc);
			}
			return result;
		}
		inline std::vector<_t*> allocate(size_t num)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return allocate_lockless(num);
		}
		inline _t* try_allocate()
		{
			if (!m_mutex.try_lock()) return nullptr;
			_t* result = allocate_lockless();
			m_mutex.unlock();
			return result;
		}

		inline bool free_lockless(_t*& pointer)
		{
			const size_t index = get_index(pointer);
			m_freelist.push_back(index);
			return true;
		}
		inline bool free(_t*& pointer)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return free_lockless(pointer);
		}
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
			return m_freelist.size();
		}

		inline size_t get_index(_t* pointer)
		{
			assert(pointer != nullptr);
			const size_t index = (pointer - m_data);
			assert(index < _c);
			return index;
		}
		inline _t& get_data_at(size_t index)
		{
			assert(index < _c);
			return m_data[index];
		}

		void reset()
		{
			m_freelist.clear();

			for (size_t i = 0u; i < _c; ++i)
				m_freelist.push_back(i);
		}

	private:
		_t m_data[_c]{};
		std::list<size_t> m_freelist{};
		std::mutex m_mutex{};
	};
}