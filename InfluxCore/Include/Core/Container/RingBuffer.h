#pragma once

#ifndef _CORE_RINGBUFFER_H_
#define _CORE_RINGBUFFER_H_

#include <mutex>
#include "Core/Math/Math.h"
#include "Core/Function.h"

namespace influx
{
	namespace detail
	{
		using capacity_t = size_t;
	}

	// FIFO queue really...
	template <typename _t, detail::capacity_t _C>
	class ringbuffer
	{
	public:
		ringbuffer() = default;
		bool push(const _t& value);
		bool pop(_t& value);
		bool pop_if(_t& value, const function<bool(const _t&)> cond);
		bool peak(detail::capacity_t i, _t& value);
		
		// push, if (full) => pop
		_t* pop_to_push(const _t& value);

		detail::capacity_t size() const;

		_t get_average_value(const detail::capacity_t num_elements = _c);

	private:
		_t m_data[_C]{};
		detail::capacity_t m_head = 0;
		detail::capacity_t m_tail = 0;
		std::mutex m_lock{};
	};

	template<typename _t, detail::capacity_t _C>
	inline bool ringbuffer<_t, _C>::push(const _t& value)
	{
		bool result = false;

		m_lock.lock();
		detail::capacity_t next = (m_head + 1u) % _C;
		if (next != m_tail)
		{
			m_data[m_head] = value;
			m_head = next;
			result = true;
		}
		m_lock.unlock();

		return result;
	}

	template<typename _t, detail::capacity_t _C>
	inline bool ringbuffer<_t, _C>::pop(_t& value)
	{
		bool result = false;

		m_lock.lock();
		if (m_tail != m_head)
		{
			value = m_data[m_tail];
			m_tail = (m_tail + 1) % _C;
			result = true;
		}
		m_lock.unlock();

		return result;
	}

	template<typename _t, detail::capacity_t _C>
	inline bool ringbuffer<_t, _C>::pop_if(_t& value, const function<bool(const _t&)> cond)
	{
		bool result = false;

		m_lock.lock();
		if (m_tail != m_head)
		{
			value = m_data[m_tail];
			if (cond(value) == true)
			{
				m_tail = (m_tail + 1) % _C;
				result = true;
			}
		}
		m_lock.unlock();

		return result;
	}

	template<typename _t, detail::capacity_t _C>
	bool ringbuffer<_t, _C>::peak(detail::capacity_t i, _t& value)
	{
		if (i >= size())
		{
			return false;
		}

		m_lock.lock();
		value = m_data[(m_tail + i) % _C];
		m_lock.unlock();

		return true;
	}

	template<typename _t, detail::capacity_t _C>
	_t* ringbuffer<_t, _C>::pop_to_push(const _t& value)
	{
		_t* result = nullptr;

		// try push
		if (!push(value))
		{
			m_lock.lock();
			detail::capacity_t next = (m_head + 1) % _C;
			if (next != m_tail)
			{
				m_data[m_head] = value;
				m_head = next;
				result = nullptr;
			}
			else
			{
				result = &m_data[m_tail];
				m_tail = (m_tail + 1u) % _C;
				m_head = (m_head + 1u) % _C;
			}
			m_lock.unlock();

			return result;
		}

		return nullptr;
	}

	template<typename _t, detail::capacity_t _C>
	inline detail::capacity_t ringbuffer<_t, _C>::size() const
	{
		return (m_head - m_tail) % _C;
	}

	template<typename _t, detail::capacity_t _c>
	inline _t ringbuffer<_t, _c>::get_average_value(const detail::capacity_t num_elements)
	{
		_t result{};
		detail::capacity_t num = math::minimum(num_elements, size());
		if (num <= 0u) return result;

		m_lock.lock();
		for (detail::capacity_t i = m_head; i != (m_head - num) % _c; i = (i - 1u) % _c)
		{
			result += m_data[i];
		}
		m_lock.unlock();

		result /= static_cast<float>(num);
		return result;
	}
}

#endif