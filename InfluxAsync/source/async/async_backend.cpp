#include "async_pch.h"
#include "async_backend.h"

#include "async/task_data.h"
#include "async/task_queue.h"

namespace influx::async
{
	void async_manager::initialize(const init_args& args)
	{
		m_is_initialized = true;

		// create queues & pool
		mp_global_cleanup_queue = new task_queue();
		mp_global_queue = new task_queue();
		mp_taskpool = new task_pool();

		// spin worker threads:
		for (uint64 i = 0u; i < args.m_num_workers; ++i)
		{
			m_worker_threads.push_back({i});
			m_worker_threads.back().m_thread_obj = 
				std::thread(&async_manager::worker_thread_method, std::ref(m_worker_threads[i]));
		}

#if _DEBUG
		gp_global_manager_state = this;
#endif
	}

	void async_manager::shutdown()
	{
		// join the threads
		for (uint64 i = 0u; i < m_worker_threads.size(); ++i)
		{
			if (m_worker_threads[i].m_thread_obj.joinable())
				m_worker_threads[i].m_thread_obj.join();
		}

		// delete objects
		influx_delete(mp_global_cleanup_queue);
		influx_delete(mp_global_queue);
		influx_delete(mp_taskpool);

		m_is_initialized = false;
	}

	void async_manager::worker_thread_method(worker_state& state)
	{
		async_manager& manager = async_manager::get_instance();
		task_queue& global_queue = manager.get_global_queue();
		task_queue& global_cleanup = manager.get_global_cleanup_queue();
		task_data* cur_task_data = nullptr;
		task_handle* cur_task_handle = nullptr;

		while (true)
		{
			if (!manager.grab_and_process_a_task())
			{
				// if we failed to grab/process a task, 
				// try cleaning one up instead...
				manager.grab_and_clean_a_task();
			}
		}
	}

	bool async_manager::grab_and_clean_a_task()
	{
		influx_assert_not_null(mp_global_cleanup_queue);

		task_data* task = nullptr;
		if (mp_global_cleanup_queue->try_grab(task))
		{
			do_cleanup_task(task);
			return true;
		}

		return false;
	}

	bool async_manager::grab_and_process_a_task()
	{
		influx_assert_not_null(mp_global_queue);

		task_data* task = nullptr;
		if (mp_global_queue->try_grab(task))
		{
			do_process_task(task);
			return true;
		}

		return false;
	}


	task_handle async_manager::create_task(const task_create_args& args)
	{
		return create_tasks({ args }).front();
	}

	std::vector<task_handle> async_manager::create_tasks(const std::vector<task_create_args>& args)
	{
		std::vector<task_handle> result{};
		for (task_data* new_data : mp_taskpool->allocate(args.size()))
		{
			influx_assert_not_null(new_data);

			// reset to initialize timers
			new_data->reset(e_task_state::allocated);

			result.push_back(task_handle(mp_taskpool->get_index(new_data)));
		}
		return result;
	}


	void async_manager::do_process_task(task_data* data)
	{
		data->m_time_started = time::get_now();
		data->m_state = e_task_state::running;
		{
			// do the work
			if (data->m_args.m_func_execute)
				data->m_args.m_func_execute();
		}
		data->m_state = e_task_state::finished;
		data->m_time_finished = time::get_now();

		// add to cleanup queue
		mp_global_cleanup_queue->push(data);
	}

	void async_manager::do_cleanup_task(task_data* data)
	{
		influx_assert_not_null(data);

		// free from task_pool
		if (mp_taskpool->free(data))
		{
			return;
		}
		
		influx_assert(false);
	}

	void async_manager::do_cleanup_task(const task_handle& handle)
	{
		do_cleanup_task(get_task_from_handle(handle));
	}

	task_data* async_manager::get_task_from_handle(const task_handle& handle)
	{
		assert(handle.is_valid());
		auto& data = mp_taskpool->get_data_at(handle.m_task_data_idx);
		
		if (data.m_state == e_task_state::invalid)
		{
			return nullptr;
		}
		else
		{
			return &data;
		}
	}

	void async_manager::dispatch(const task_handle& handle)
	{
		task_data* data = get_task_from_handle(handle);
		influx_assert_not_null(data);

		dispatch(data);
	}

	void async_manager::dispatch(task_data* data)
	{
		data->set_state(e_task_state::pending);
		influx_assert(mp_global_queue->push(data));
	}

	void async_manager::wait_for(const task_handle& handle, const wait_args& args)
	{
		wait_for(vector<task_handle>{ handle });
	}

	void async_manager::wait_for(const vector<task_handle>& handles, const wait_args& args)
	{
		time::point wait_start = time::get_now();
		float ms_waited = 0.0f;
		for (const task_handle& handle : handles)
		{
			influx_assert(handle.is_valid());

			task_data* data = get_task_from_handle(handle);
			influx_assert_not_null(data);

			while (!data->is_finished() && ms_waited < args.m_max_ms)
			{
				// might as well clean up a bit
				grab_and_clean_a_task();

				if (args.m_wait_func)
				{
					args.m_wait_func();
				}
				
				ms_waited = time::get_ms_between<float>(time::get_now(), wait_start);
			}
		}

		if (args.mp_out_ms != nullptr)
		{
			(*args.mp_out_ms) = ms_waited;
		}
	}

	task_queue& async_manager::get_global_queue()
	{
		return *mp_global_queue;
	}

	task_queue& async_manager::get_global_cleanup_queue()
	{
		return *mp_global_cleanup_queue;
	}

#pragma region frontend_api
	constexpr uint64 get_static_num_bytes()
	{
		return sizeof(async_manager::get_instance());
	}
	void initialize(const init_args& args)
	{
		return async_manager::get_instance().initialize(args);
	}
	task_handle create_task(const task_create_args& args)
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
	void wait_for(const vector<task_handle>& handles, const wait_args& args)
	{
		async_manager::get_instance().wait_for(handles, args);
	}
	void shutdown()
	{
		async_manager::get_instance().shutdown();
	}

	// [task handle]
	task_handle::task_handle(size_t data_idx)
		: m_task_data_idx{ data_idx }
	{
		task_data* data = get_task_data();
		if (data != nullptr)
		{
			data->m_refcount++;
		}
	}
	task_handle::task_handle(const task_handle& other)
	{
		m_task_data_idx = other.m_task_data_idx;
		task_data* data = get_task_data();
		if (data != nullptr)
		{
			data->m_refcount++;
		}
	}
	task_handle::task_handle(task_handle&& other) noexcept
	{
		m_task_data_idx = other.m_task_data_idx;
		task_data* data = get_task_data();
		if (data != nullptr)
		{
			data->m_refcount++;
		}
	}
	task_handle& task_handle::operator=(const task_handle& other)
	{
		m_task_data_idx = other.m_task_data_idx;
		task_data* data = get_task_data();
		if (data != nullptr)
		{
			data->m_refcount++;
		}
		return *this;
	}
	task_handle& task_handle::operator=(task_handle&& other) noexcept
	{
		m_task_data_idx = other.m_task_data_idx;
		task_data* data = get_task_data();
		if (data != nullptr)
		{
			data->m_refcount++;
		}
		return *this;
	}
	bool task_handle::is_valid() const
	{
		return (m_task_data_idx != k_task_invalid_idx);
	}
	bool task_handle::is_equal(const task_handle& other) const
	{
		return m_task_data_idx == other.m_task_data_idx;
	}
	e_task_state task_handle::get_state() const
	{
		influx_assert(is_valid());
		task_data* data = get_task_data();
		return (data != nullptr) ? data->m_state : e_task_state::invalid;
	}
	task_stats task_handle::get_stats() const
	{
		influx_assert(is_valid());
		task_data* data = get_task_data();
		return (data != nullptr) ? data->m_stats : task_stats{};
	}
	void task_handle::wait(const wait_args& args) const
	{
		async::async_manager::get_instance().wait_for(*this, args);
	}
	bool task_handle::is_finished() const
	{
		return get_state() == e_task_state::finished;
	}
	void task_handle::dispatch() const
	{
		async::async_manager::get_instance().dispatch(*this);
	}
	bool task_handle::operator==(const task_handle& other) const
	{
		return is_equal(other);
	}
	bool task_handle::operator!=(const task_handle& other) const
	{
		return !is_equal(other);
	}
	task_data* task_handle::get_task_data() const
	{
		return async_manager::get_instance().get_task_from_handle(*this);
	}
#pragma endregion
}