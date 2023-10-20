#include "async_pch.h"
#include "async_manager.h"

namespace influx::async
{
	task_handle::task_handle(task_id id)
		: m_id{id}
	{

	}

	bool task_handle::is_valid() const
	{
		return m_id != k_task_id_invalid;
	}

	bool task_handle::is_equal(const task_handle& other) const
	{
		return m_id == other.m_id;
	}

	bool task_handle::operator==(const task_handle& other) const
	{
		return is_equal(other);
	}
	bool task_handle::operator!=(const task_handle& other) const
	{
		return !is_equal(other);
	}
}

namespace influx::async
{
	void async_manager::initialize(const init_args& args)
	{
		m_is_initialized = true;

		// spin worker threads:
		for (uint64 i = 0u; i < args.m_num_workers; ++i)
		{
			m_worker_states.push_back(worker_state{ i });
			m_worker_threads.push_back(std::thread(&async_manager::worker_thread_method, m_worker_states[i]));
		}
	}

	void async_manager::worker_thread_method(worker_state& state)
	{
		async_manager& manager = async_manager::get_instance();
		work_queue& global_queue = manager.get_global_queue();
		task_data* cur_task_data = nullptr;
		task_handle cur_task_handle = task_handle{ k_task_id_invalid };

		while (true)
		{
			// fetch a task from the global queue
			cur_task_handle = global_queue.m_queued_tasks.back();
			global_queue.m_queued_tasks.pop();
			cur_task_data = manager.get_task_data_from_handle(cur_task_handle);

			{
				// do the work
				cur_task_data->m_args.m_func_execute();
				cur_task_data->m_args.m_func_on_finish();
			}

			// do the cleanup
			manager.cleanup_task(cur_task_data);
		}
	}

	task_handle async_manager::create_task(const task_args& args = {})
	{
		task_data* new_task_data = m_taskpool.try_acquire();
		if (new_task_data == nullptr)
		{
			return task_handle(k_task_id_invalid);
		}

		// initialize data with args
		(*new_task_data) = task_data{ args };
		new_task_data->m_handle = task_handle(0u); // todo: create a unique handle!

		mp_taskdatas.push_back(new_task_data);
		return new_task_data->m_handle;
	}

	void async_manager::cleanup_task(const task_handle& handle)
	{
		cleanup_task(get_task_data_from_handle(handle));
	}

	void async_manager::cleanup_task(task_data* data)
	{
		if (data == nullptr)
		{
			return;
		}

		auto found = std::find_if(mp_taskdatas.cbegin(), mp_taskdatas.cend(), 
			[&data](task_data* this_data)
			{
				return this_data == data;
			});

		if (found == mp_taskdatas.cend())
		{
			return;
		}

		m_taskpool.try_release(*found);
		mp_taskdatas.erase(found);
	}

	async_manager::task_data* async_manager::get_task_data_from_handle(const task_handle& handle)
	{
		auto found = std::find_if(mp_taskdatas.cbegin(), mp_taskdatas.cend(),
			[&handle](const task_data* data)
			{
				return data->m_handle == handle;
			});

		if (found != mp_taskdatas.cend())
		{
			return *found;
		}
		else
		{
			return nullptr;
		}
	}

	void async_manager::dispatch(const task_handle& handle)
	{
		task_data* data = get_task_data_from_handle(handle);
		if (data == nullptr)
		{
			return;
		}
	}

	void async_manager::wait_for(const task_handle& handle, const wait_args& args)
	{
		task_data* data = get_task_data_from_handle(handle);
		if (data == nullptr)
		{
			return;
		}
	}

	async_manager::work_queue& async_manager::get_global_queue()
	{
		return m_global_queue;
	}

	void async_manager::shutdown()
	{
		m_is_initialized = false;

		for (uint64 i = 0u; i < m_worker_threads.size(); ++i)
		{
			m_worker_threads[i].join();
		}
	}

#pragma region frontend_api
	void initialize(const init_args& args)
	{
		return async_manager::get_instance().initialize(args);
	}

	task_handle create_task(const task_args& args = {})
	{
		return async_manager::get_instance().create_task(args);
	}

	void dispatch(const task_handle& handle)
	{
		async_manager::get_instance().dispatch(handle);
	}

	void wait_for(const task_handle& handle, const wait_args& args)
	{
		async_manager::get_instance().wait_for(handle, args);
	}

	void shutdown()
	{
		async_manager::get_instance().shutdown();
	}
#pragma endregion
}