#pragma once

#include "list.h"
#include <atomic>

namespace influx
{
	namespace
	{
		using pool_size_t = unsigned long long;
	}

	/*
	*  essentially a freelist
	* 
	*	influenced by:
	*	https://codereview.stackexchange.com/questions/79084/simple-concurrent-freelist
	*/
	template <class _t, pool_size_t _c>
	class pool final
	{
	private:
		struct node final
		{
			node() = default;
			_t mp_data{};
			std::atomic<node*> mp_next = nullptr;
		};

	public:
		inline pool()
		{
			reset();
		}

		inline _t* try_acquire()
		{
			node* current_head = m_head.load(std::memory_order_acquire);
			node* new_head = nullptr;

			while (current_head != nullptr)
			{
				// Construct the new head by grabbing next pointer in list and incrementing the atomic tag
				new_head = current_head->mp_next.load(std::memory_order_relaxed);

				if (m_head.compare_exchange_weak(current_head, new_head, std::memory_order_release, std::memory_order_acquire))
				{
					// Succesfully popped an item, return pointer to data
					return &current_head->mp_data;
				}
			}

			return nullptr;
		}

		inline bool try_release(_t* ptr)
		{
			// Check that the given pointer points inside the pool and lies along an item
			auto ptr_delta = reinterpret_cast<node*>(ptr) - reinterpret_cast<node*>(&m_nodes[0u].mp_data);
			if (ptr_delta < 0u || ptr_delta >= _c || &m_nodes[ptr_delta].mp_data != ptr)
			{
				return false;
			}
			
			node* current_head = m_head.load(std::memory_order_relaxed);
			node* new_head(&m_nodes[ptr_delta]);

			// try to replace the current head until we succeed
			do 
			{
				// make the head of the current list the new tail
				m_nodes[ptr_delta].mp_next.store(current_head, std::memory_order_relaxed);
			} while (!m_head.compare_exchange_weak(current_head, new_head, std::memory_order_release, std::memory_order_relaxed));

			// added item back to pool
			return true;
		}

		inline void reset()
		{
			m_head.store(&m_nodes[0u], std::memory_order_relaxed);

			// clear last node
			new (&m_nodes[_c - 1u]) node();

			for (pool_size_t i = 0u; i < _c - 1u; ++i)
			{
				m_nodes[i].mp_next.store(&m_nodes[i + 1u], std::memory_order_relaxed);
			}

			// Issue memory barrier so we can immediately start work
			std::atomic_thread_fence(std::memory_order_release);
		}

		inline constexpr static pool_size_t get_capacity()
		{
			return _c;
		}

	private:
		node m_nodes[_c]{};
		std::atomic<node*> m_head = nullptr;

		pool_size_t m_num_in_flight = 0;
	};
}