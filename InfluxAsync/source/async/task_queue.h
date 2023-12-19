#pragma once

#include "core/container/ringbuffer.h"
#include "core/container/vector.h"

namespace influx::async
{
	struct task_data;

	class task_queue final
	{
	public:
		inline bool try_grab(task_data*& task)
		{
			return m_buffer.try_pop(task);
		}

		inline bool push(task_data* data)
		{
			return m_buffer.push(data);
		}

		inline bool push(const vector<task_data*>& datas)
		{
			return m_buffer.push(datas);
		}

	private:
		ringbuffer<task_data*, 4096u> m_buffer{};
	};
}


