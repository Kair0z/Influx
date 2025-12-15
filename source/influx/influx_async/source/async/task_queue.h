#pragma once

#include "core/container/ringbuffer.h"
#include "core/container/vector.h"

namespace influx::async
{
	struct task_data;

	class task_queue final
	{
		ringbuffer<task_data*, 4096u> m_buffer{};

	public:
		inline bool try_grab(task_data*& task)
		{
			return m_buffer.try_pop(task);
		}

		inline bool try_grab_lockless(task_data*& task)
		{
			return m_buffer.pop_lockless(task);
		}

		inline bool push(task_data* data)
		{
			return m_buffer.push(data);
		}

		inline bool push(const vector<task_data*>& datas)
		{
			return m_buffer.push(datas);
		}

		inline uint64 size() const
		{
			return m_buffer.size();
		}
	};
}


