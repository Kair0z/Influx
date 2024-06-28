#pragma once

#include <mutex>

#if 1
#include "core/math/math.h"
#include "core/function.h"
#include "core/container/vector.h"

#else
#include <algorithm>
#include <cassert>
namespace influx
{
	template <typename _t>
	constexpr inline _t minimum(std::initializer_list<_t> list)
	{
		return std::min(list);
	}

#define influx_assert(x) assert(x);
}
#endif

namespace influx
{
	template <typename _t, size_t _c>
	class ringbuffer final
	{
	public:
		ringbuffer() = default;

		bool push_lockless(const _t& value);
		bool push_lockless(const vector<_t>& values);
		bool push(const _t& value);
		bool push(const vector<_t>& values);
		bool try_push(const _t& value);
		bool try_push(const vector<_t>& values);

		bool pop_lockless(_t& value);
		bool pop(_t& value);
		bool try_pop(_t& value);

		bool pop(std::vector<_t>& out_values, size_t num = -1);
		bool try_pop(std::vector<_t>& out_values, size_t num = -1);

		template <class _func>
		bool pop_if_lockless(_t& value, _func&& cond);
		template <class _func>
		bool pop_if(_t& value, _func&& cond);
		template <class _func>
		bool try_pop_if(_t& value, _func&& cond);

		bool peak_lockless(size_t i, _t& value);
		bool peak(size_t i, _t& value);
		bool try_peak(size_t i, _t& value);

		// push, if (full) => pop
		void pop_to_push_lockless(const _t& value, _t* out_popped = nullptr);
		void pop_to_push(const _t& value, _t* out_popped = nullptr);

		size_t size() const;
		constexpr static size_t capacity();
		bool is_full() const;
		bool is_empty() const;

		template <class _func>
		void for_each_lockless(_func&& func, const size_t max_num = -1);
		template <class _func>
		void for_each(_func&& func, const size_t max_num = -1);
		template <class _func>
		bool try_for_each(_func&& func, const size_t max_num = -1);

		_t get_average_value(const size_t num_elements = _c);

	private:
		_t m_data[_c]{};
		size_t m_head = 0;
		size_t m_tail = 0;
		std::mutex m_lock{};

		_t m_cached_sum{};
	};

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::push_lockless(const _t& value)
	{
		const size_t next = (m_head + 1u) % _c;
		if (next != m_tail)
		{
			m_data[m_head] = value;
			m_head = next;
			return true;
		}
		
		return false;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::push_lockless(const vector<_t>& values)
	{
		bool result = false;
		for (size_t i = 0u; i < values.size(); ++i)
		{
			result |= push_lockless(values[i]);
		}
		return result;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::push(const _t& value)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return push_lockless(value);
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::push(const vector<_t>& values)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return push_lockless(values);
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::try_push(const _t& value)
	{
		if (!m_lock.try_lock()) return false;
		bool result = push_lockless(value);
		m_lock.unlock();
		return result;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::try_push(const vector<_t>& values)
	{
		if (!m_lock.try_lock()) return false;
		bool result = push_lockless(values);
		m_lock.unlock();
		return result;
	}


	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::pop_lockless(_t& value)
	{
		if (m_tail != m_head)
		{
			value = m_data[m_tail];
			m_tail = (m_tail + 1) % _c;
			return true;
		}

		return false;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::pop(_t& value)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return pop_lockless(value);
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::try_pop(_t& value)
	{
		if (!m_lock.try_lock()) return false;
		bool result = pop_lockless(value);
		m_lock.unlock();
		return result;
	}

	template<typename _t, size_t _c>
	bool ringbuffer<_t, _c>::pop(std::vector<_t>& out_values, size_t num)
	{
		num = math::minimum(num, size());
		out_values.resize(num);
		std::lock_guard<std::mutex> lock(m_lock);

		for (size_t i = 0u; i < num; ++i)
		{
			if (!pop_lockless(out_values[i]))
			{
				return false;
			}
		}

		return true;
	}

	template<typename _t, size_t _c>
	bool ringbuffer<_t, _c>::try_pop(std::vector<_t>& out_values, size_t num)
	{
		num = math::minimum(num, size());
		out_values.resize(num);
		
		if (!m_lock.try_lock()) return false;

		for (size_t i = 0u; i < num; ++i)
		{
			if (!pop_lockless(out_values[i]))
			{
				return false;
			}
		}

		m_lock.unlock();
		return true;
	}


	template<typename _t, size_t _c>
	template <class _func>
	inline bool ringbuffer<_t, _c>::pop_if_lockless(_t& value, _func&& cond)
	{
		if (cond() == false) 
			return false;

		return pop_lockless(value);
	}

	template<typename _t, size_t _c>
	template <class _func>
	inline bool ringbuffer<_t, _c>::pop_if(_t& value, _func&& cond)
	{
		std::lock_guard<std::mutex>(m_lock);
		return pop_if_lockless(value, cond);
	}

	template<typename _t, size_t _c>
	template <class _func>
	inline bool ringbuffer<_t, _c>::try_pop_if(_t& value, _func&& cond)
	{
		if (!m_lock.try_lock()) return false;
		bool result = pop_if_lockless(value, cond);
		m_lock.unlock();
		return result;
	}


	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::peak_lockless(size_t i, _t& value)
	{
		influx_assert(i < size());
		value = m_data[(m_tail + i) % _c];
		return true;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::peak(size_t i, _t& value)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return peak_lockless(i, value);
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::try_peak(size_t i, _t& value)
	{
		if (!m_lock.try_lock()) return false;
		bool result = peak_lockless(i, value);
		m_lock.unlock();
		return result;
	}


	template<typename _t, size_t _c>
	inline void ringbuffer<_t, _c>::pop_to_push_lockless(const _t& value, _t* out_popped)
	{
		_t popped{};
		while (!push_lockless(value)) // as long as push failed
		{
			pop_lockless(popped);
		}

		if (out_popped)
		{
			*out_popped = popped;
		}
	}

	template<typename _t, size_t _c>
	inline void ringbuffer<_t, _c>::pop_to_push(const _t& value, _t* out_popped)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return pop_to_push_lockless(value, out_popped);
	}


	template<typename _t, size_t _c>
	inline size_t ringbuffer<_t, _c>::size() const
	{
		return (m_head - m_tail) % _c;
	}

	template<typename _t, size_t _c>
	inline constexpr size_t ringbuffer<_t, _c>::capacity()
	{
		return _c;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::is_full() const
	{
		return ((m_head + 1u) % _c) == m_tail;
	}

	template<typename _t, size_t _c>
	inline bool ringbuffer<_t, _c>::is_empty() const
	{
		return m_head == m_tail;
	}

	template<typename _t, size_t _c>
	template <class _func>
	inline void ringbuffer<_t, _c>::for_each_lockless(_func&& func, const size_t max_num)
	{
		const size_t num = math::minimum(max_num, size());
		const size_t tail = (m_head - num) % _c;
		for (size_t i = m_head; i != tail; i = (i - 1u) % _c)
		{
			func(m_data[i]);
		}
	}

	template<typename _t, size_t _c>
	template <class _func>
	void ringbuffer<_t, _c>::for_each(_func&& func, const size_t max_num)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		for_each_lockless(func, max_num);
	}

	template<typename _t, size_t _c>
	template <class _func>
	bool ringbuffer<_t, _c>::try_for_each(_func&& func, const size_t max_num)
	{
		if (!m_lock.try_lock()) return false;
		for_each_lockless(func, max_num);
		m_lock.unlock();
		return true;
	}


	template<typename _t, size_t _c>
	inline _t ringbuffer<_t, _c>::get_average_value(const size_t num_elements)
	{
		_t result{};
		size_t num = math::minimum(num_elements, size());
		for_each([&result](const _t& val)
		{
			result += val;
		}, num);

		result /= static_cast<float>(num);

		return result;
	}
}