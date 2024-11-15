#pragma once

#include "core/debug.h"

#include <queue>
#include <atomic>
#include <queue>
#include <mutex>
#include <assert.h>

namespace influx
{
	// WIP
#if 0
	namespace detail
	{
		class ipool
		{
		public:
			virtual void reset() = 0;

			// get number of bytes
			virtual uint64 get_bytesize() const = 0;

			// get max number of elements
			virtual uint32 get_capacity() const = 0;

			virtual uint32 get_num_free() const = 0;

			// get index of pointer if it belongs
			virtual uint64 get_element_index(void*) const = 0;

			virtual bool owns(void*) const = 0;

			// allocating elements
			template <typename _t>
			_t** allocate_lockless(uint64 num)
			{
				return static_cast<_t**>(allocate_lockless(num));
			}

			template <typename _t>
			_t** allocate(uint64 num)
			{
				return static_cast<_t**>(allocate_lockless(num));
			}

			template <typename _t>
			_t* allocate_lockless()
			{
				return static_cast<_t*>(allocate_lockless());
			}

			template <typename _t>
			_t* allocate();

			template <typename _t>
			const _t& at(uint64 idx) const
			{
				if (idx < get_capacity())
				{

				}
			}

			template <typename _t>
			_t& at(uint64 idx)
			{
				if (idx < get_capacity())
				{
					
				}
			}

			virtual void** allocate_lockless(uint64 num) = 0;

			virtual void** allocate(uint64 num) = 0;

			virtual void* allocate_lockless() = 0;

			virtual void* allocate() = 0;
			
			virtual void* try_allocate() = 0;

			// freeing elements
			virtual bool free_lockless(void*& ptr) = 0;

			virtual bool free(void*& ptr) = 0;

			virtual bool try_free(void*& ptr) = 0;

			virtual void* at(uint64 idx);

			virtual ~ipool() = default;

		protected:
			std::mutex m_mutex{};
		};
	}

	// _t: data type
	// _c: capacity
	template <class _t, uint64 _c>
	class pool final : public detail::ipool
	{
	public:
		pool() : ipool()
		{
			reset();
		}
		pool(const pool& other) = delete;
		pool(pool&& other) = delete;

		virtual void reset() override
		{
			m_freelist.clear();

			for (uint64 i = 0u; i < _c; ++i)
				m_freelist.push_back(i);
		}

		virtual void get_bytesize() const override
		{
			return _c * sizeof(_t);
		}

		virtual uint64 get_capacity() const override
		{
			return _c;
		}

		virtual uint64 get_num_free() const override
		{
			return m_freelist.size();
		}

		virtual uint64 get_element_index(void* pointer_void) const override
		{
			assert(pointer_void != nullptr);
			const size_t offset = (pointer - m_data);
			assert(offset < _c);
			return offset;
		}

		virtual bool owns(void* pointer_void) const override
		{
			return (get_element_index() < _c);
		}

		virtual void** allocate_lockless(uint64 num) override
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

		virtual void** allocate(uint64 num) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return allocate_lockless(num);
		}

		virtual void* allocate_lockless() override
		{
			assert(get_num_free() > 0u);

			const size_t free_index = m_freelist.back();
			m_freelist.pop_back();

			return &m_data[free_index];
		}

		virtual void* allocate() override
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return allocate_lockless();
		}

		virtual void* try_allocate() override
		{
			if (!m_mutex.try_lock()) return nullptr;
			_t* result = allocate_lockless();
			m_mutex.unlock();
			return result;
		}

		virtual bool free_lockless(void*& pointer) override
		{
			const size_t index = get_element_index(pointer);
			m_freelist.push_back(index);
			return true;
		}

		virtual bool free(void*& pointer) override
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return free_lockless(pointer);
		}

		virtual bool try_free(void*& pointer) override
		{
			if (!m_mutex.try_lock()) return nullptr;
			bool result = free_lockless(pointer);
			m_mutex.unlock();
			return result;
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

	private:
		_t m_data[_c]{};
		std::list<size_t> m_freelist{};
		
	};

	template <typename _t>
#endif

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

		inline bool free_lockless(_t* pointer)
		{
			const size_t index = get_index(pointer);
			m_freelist.push_back(index);
			return true;
		}
		inline bool free(_t* pointer)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return free_lockless(pointer);
		}
		inline bool try_free(_t* pointer)
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