#include "async_pch.h"
#include "async_backend.h"

#include "async/task_data.h"
#include "async/task_queue.h"

namespace influx::async
{
	result<> async_manager::initialize(const init_args& args)
	{
		m_is_initialized = true;

		// create queues & pool
		mp_global_cleanup_queue = new task_queue();
		mp_global_queue			= new task_queue();
		mp_taskpool				= new task_pool();

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
		return {};
	}

	result<> async_manager::shutdown()
	{
		m_is_initialized = false;

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

		return {};
	}

	bool async_manager::is_shutdown()
	{
		return m_is_initialized == false;
	}

	void async_manager::worker_thread_method(worker_state& state)
	{
		// routine:
		// 1. check if manager is still active
		// 2. grab/process a task, 
		// 3. if no task was processed, try cleaning one up instead

		async_manager& manager = async_manager::get_instance();
		while (manager.is_shutdown() == false)
		{
			const bool processed_task = manager.try_grab_and_process_a_task(manager.get_global_queue()).get();
			if (!processed_task)
			{
				manager.try_grab_and_clean_a_task();
			}
		}
	}

	result<bool> async_manager::try_grab_and_clean_a_task()
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

	result<bool> async_manager::try_grab_and_process_a_task(task_queue& queue)
	{
		using result_type = result<bool>;

		task_data* task = nullptr;
		if (queue.try_grab(task))
		{
			// grabbed a task, now process (execute it)
			auto res = do_process_task(task);
			return res.is_success();
		}

		// failed to grab a task
		return false;
	}

	result<task_handle> async_manager::create_task(const task_create_args& args)
	{
		using result_type = result<task_handle>;

		if (m_is_initialized == false)
			return result_type::make_error("async API not intialized!");

		return create_tasks({ args }).get().front();
	}

	result<std::vector<task_handle>> async_manager::create_tasks(const std::vector<task_create_args>& args)
	{
		std::vector<task_handle> result{};
		vector<task_data*> allocated_tasks = mp_taskpool->allocate(args.size());

		influx_assert(allocated_tasks.size() == args.size());

		for (uint64 i = 0u; i < allocated_tasks.size(); ++i)
		{
			task_data* new_task_data = allocated_tasks[i];
			influx_assert_not_null(new_task_data);

			// initialize the allocated task
			new_task_data->reset(e_task_state::allocated);
			new_task_data->m_args = args[i];

			// push handle to data
			result.push_back(task_handle(mp_taskpool->get_index(new_task_data)));
		}
		return result;
	}

	result<> async_manager::do_process_task(task_data* data)
	{
		data->m_time_started = time::get_now();
		data->m_state = e_task_state::running;
		m_num_processing++;
		{
			// do the work
			if (data->m_args.m_func_execute)
				data->m_args.m_func_execute();
		}
		m_num_processing--;
		data->m_state = e_task_state::finished;
		data->m_time_finished = time::get_now();

		// add to cleanup queue
		mp_global_cleanup_queue->push(data);

		return {};
	}

	result<> async_manager::do_cleanup_task(task_data* data)
	{
		influx_assert_not_null(data);

		// free from task_pool
		if (mp_taskpool->free(data))
		{
			return {};
		}
		
		influx_assert(false);
		return result<>::make_error("clean failed!");
	}

	result<> async_manager::do_cleanup_task(const task_handle& handle)
	{
		return do_cleanup_task(get_task_from_handle(handle).get());
	}

	result<task_data*> async_manager::get_task_from_handle(const task_handle& handle)
	{
		using result_type = result<task_data*>;

		assert(handle.is_valid());
		auto& data = mp_taskpool->get_data_at(handle.m_task_data_idx);
		
		if (data.m_state != e_task_state::invalid)
		{
			return &data;
		}
		else
		{
			return result_type::make_error("data at given handle is invalid!");
		}
	}

	result<uint64> async_manager::get_num_queued() const
	{
		return mp_global_queue->size();
	}

	result<uint64> async_manager::get_num_processing() const
	{
		return m_num_processing.load();
	}

	result<uint64> async_manager::get_num_toclean() const
	{
		return mp_global_cleanup_queue->size();
	}

	result<bool> async_manager::has_unfinished_work() const
	{
		return (get_num_queued() > 0u || get_num_processing() > 0u);
	}

	result<> async_manager::dispatch(const task_handle& handle)
	{
		task_data* data = get_task_from_handle(handle).get();
		influx_assert_not_null(data);

		return dispatch(data);
	}

	result<> async_manager::dispatch(task_data* data)
	{
		data->set_state(e_task_state::queued);
		influx_assert(mp_global_queue->push(data));
		return {};
	}

	result<> async_manager::wait_for(const task_handle& handle, const wait_args& args)
	{
		return wait_for(vector<task_handle>{ handle });
	}

	result<> async_manager::wait_for(const vector<task_handle>& handles, const wait_args& args)
	{
		time::point wait_start = time::get_now();
		float ms_waited = 0.0f;
		for (const task_handle& handle : handles)
		{
			influx_assert(handle.is_valid());

			task_data* data = get_task_from_handle(handle).get();
			influx_assert_not_null(data);

			while (!data->is_finished() && ms_waited < args.m_max_ms)
			{
				// might as well clean up a bit
				try_grab_and_clean_a_task();

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
		return {};
	}

	result<>  async_manager::wait_for_all(const wait_args& args)
	{
		time::point wait_start = time::get_now();
		float ms_waited = 0.0f;

		while (has_unfinished_work() && ms_waited < args.m_max_ms)
		{
			if (args.m_wait_func)
				args.m_wait_func();

			ms_waited = time::get_ms_between<float>(time::get_now(), wait_start);
		}
		
		if (args.mp_out_ms != nullptr)
		{
			(*args.mp_out_ms) = ms_waited;
		}
		return {};
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
	result<> initialize(const init_args& args)
	{
		return async_manager::get_instance().initialize(args);
	}
	result<task_handle> create_task(const task_create_args& args)
	{
		return async_manager::get_instance().create_task(args);
	}
	result<> dispatch(const task_handle& handle)
	{
		return async_manager::get_instance().dispatch(handle);
	}
	result<> wait_for(const task_handle& handle, const wait_args& args)
	{
		return async_manager::get_instance().wait_for(handle, args);
	}
	result<> wait_for(const vector<task_handle>& handles, const wait_args& args)
	{
		return async_manager::get_instance().wait_for(handles, args);
	}
	result<> wait_for_all(const wait_args& args)
	{
		return async_manager::get_instance().wait_for_all(args);
	}
	result<> shutdown()
	{
		return async_manager::get_instance().shutdown();
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

	task_handle::task_handle()
		: m_task_data_idx{(uint64)-1}
	{
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
		return async_manager::get_instance().get_task_from_handle(*this).get();
	}
#pragma endregion
}