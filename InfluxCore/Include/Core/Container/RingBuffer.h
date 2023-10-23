#pragma once

#ifndef _CORE_RINGBUFFER_H_
#define _CORE_RINGBUFFER_H_

#include <mutex>

namespace influx
{
	namespace detail
	{
		using capacity_t = size_t;
	}

	template <typename _t, detail::capacity_t _C>
	class ringbuffer
	{
	public:
		ringbuffer() = default;
		bool push(const _t& value);
		bool pop(_t& value);
		bool peak(detail::capacity_t i, _t& value);

		detail::capacity_t size() const;

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
		detail::capacity_t next = (m_head + 1) % _C;

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
	inline detail::capacity_t ringbuffer<_t, _C>::size() const
	{
		return (m_head - m_tail) % _C;
	}
}

#endif