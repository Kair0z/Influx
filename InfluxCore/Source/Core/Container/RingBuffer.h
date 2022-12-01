#pragma once

#ifndef _CORE_RINGBUFFER_H_
#define _CORE_RINGBUFFER_H_

#include <mutex>

namespace Influx
{
	namespace
	{
		using capacity_t = size_t;
	}

	template <typename _T, capacity_t _C>
	class RingBuffer
	{
	public:
		bool PushBack(const _T& value);
		bool PopFront(_T& value);

	private:
		_T m_data[_C];
		capacity_t m_head = 0;
		capacity_t m_tail = 0;
		std::mutex m_lock;
	};

	template<typename _T, capacity_t _C>
	inline bool RingBuffer<_T, _C>::PushBack(const _T& value)
	{
		bool result = false;

		m_lock.lock();
		capacity_t next = (m_head + 1) % _C;

		if (next != m_tail)
		{
			m_data[m_head] = value;
			m_head = next;
			result = true;
		}

		m_lock.unlock();

		return result;
	}

	template<typename _T, capacity_t _C>
	inline bool RingBuffer<_T, _C>::PopFront(_T& value)
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
}

#endif