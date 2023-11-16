#include "async_pch.h"
#include "async_manager.h"

#include "Core/Time.h"

namespace influx::async
{
	task_handle::task_handle(task_id id)
		: m_id{id}
	{

	}

	bool task_handle::is_valid() const
	{
		return m_id != k_task_id_invalid && mp_data != nullptr;
	}

	bool task_handle::is_equal(const task_handle& other) const
	{
		return m_id == other.m_id && mp_data == other.mp_data;
	}

	e_task_state task_handle::get_state() const
	{
		if (!is_valid())
		{
			return e_task_state::invalid;
		}
		
		return mp_data->m_state;
	}

	task_stats task_handle::get_stats() const
	{
		if (!is_valid())
		{
			return task_stats{};
		}

		task_stats stats = mp_data->m_stats;
		return stats;
	}

	bool task_handle::has_parent() const
	{
		return async::async_manager::get_instance().has_parent(*this);
	}

	void task_handle::wait(const wait_args& args) const
	{
		async::async_manager::get_instance().wait_for(*this, args);
	}

	bool task_handle::is_finished_self() const
	{
		return mp_data->is_self_finished();
	}

	bool task_handle::are_children_finished() const
	{
		return mp_data->are_children_finished();
	}

	bool task_handle::is_finished_all() const
	{
		return mp_data->is_all_finished();
	}

	void task_handle::dispatch() const
	{
		async::async_manager::get_instance().dispatch(*this);
	}

	void task_handle::add_child(const task_handle& child)
	{
		async::async_manager::get_instance().add_child(*this, child);
	}

	void task_handle::set_requeue_condition(const function<bool()>& condition_func)
	{
		async::async_manager::get_instance().set_requeue_condition(*this, condition_func);
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
			m_worker_threads.push_back(std::thread(std::bind(&async_manager::worker_thread_method, m_worker_states[i])));
		}

#if _DEBUG
		gp_global_manager_state = this;
#endif
	}

	// MEAT AND POTATOES
	void async_manager::worker_thread_method(worker_state& state)
	{
		async_manager& manager = async_manager::get_instance();
		work_queue& global_queue = manager.get_global_queue();
		work_queue& global_cleanup = manager.get_global_cleanup_queue();
		task_data* cur_task_data = nullptr;
		task_handle* cur_task_handle = nullptr;

		while (true)
		{
			if (manager.try_process_a_task())
				continue;

			// if we couldn't process a task, try cleaning up one instead...
			manager.try_cleanup_a_task();
		}
	}

	task_handle async_manager::create_task(const task_args& args)
	{
		task_data* new_task_data = m_taskpool.try_acquire();
		if (new_task_data == nullptr)
		{
			return task_handle(k_task_id_invalid);
		}

		// initialize data with args
		(*new_task_data) = task_data{ args };
		new_task_data->m_handle = task_handle(0u); // todo: create a unique handle!

		m_cleanup_mutex.lock();
		mp_taskdatas.push_back(new_task_data);
		m_cleanup_mutex.unlock();

		new_task_data->m_time_created = time::get_now();

		new_task_data->m_handle.mp_data = new_task_data;
		return new_task_data->m_handle;
	}

	bool async_manager::try_cleanup_a_task()
	{
		task_data* data_to_cleanup = nullptr;
		if (m_global_cleanup_queue.m_tasks.pop(data_to_cleanup))
		{
			cleanup_task(data_to_cleanup);
			return true;
		}

		return false;
	}

	bool async_manager::try_process_a_task()
	{
		auto is_parent_finished = [](task_data* const& data)
		{
			return (data->m_parent == nullptr) || (data->m_parent->is_self_finished());
		};

		task_data* task = nullptr;
		if (m_global_queue.m_tasks.pop_if(task, is_parent_finished))
		{
			process_task(task);
			return true;
		}

		return false;
	}

	void async_manager::cleanup_task(const task_handle& handle)
	{
		cleanup_task(get_task_data_from_handle(handle));
	}

	// MEAT AND POTATOES
	void async_manager::process_task(task_data* data)
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

		// update own counter
		data->m_num_children_unfinished--;

		// update parent counter
		if (data->m_parent != nullptr)
			data->m_parent->m_num_children_unfinished--;

		// add to cleanup queue
		m_global_cleanup_queue.m_tasks.push(data);
	}

	void async_manager::cleanup_task(task_data* data)
	{
		if (data == nullptr)
		{
			return;
		}

		{
			std::lock_guard lock{ m_cleanup_mutex };

			auto found = std::find_if(mp_taskdatas.cbegin(), mp_taskdatas.cend(),
				[&data](task_data* this_data)
				{
					return this_data == data;
				});

			if (found == mp_taskdatas.cend())
			{
				return;
			}

			mp_taskdatas.erase(found);
		}

		// todo: probably isn't safe!!!
		data->m_handle.mp_data = nullptr;
		m_taskpool.try_release(data);
	}


	task_data* async_manager::get_task_data_from_handle(const task_handle& handle)
	{
		return handle.mp_data;
	}

	void async_manager::dispatch(const task_handle& handle)
	{
		task_data* data = get_task_data_from_handle(handle);
		if (data == nullptr)
		{
			return;
		}

		dispatch(data);
	}

	void async_manager::dispatch(task_data* data)
	{
		data->reset(e_task_state::pending);
		m_global_queue.m_tasks.push(data);

		for (task_data* child : data->m_children)
		{
			dispatch(child);
		}
	}

	void async_manager::wait_for(const task_handle& handle, const wait_args& args)
	{
		if (!handle.is_valid())
		{
			return;
		}

		task_data* data = get_task_data_from_handle(handle);
		if (data == nullptr)
		{
			return;
		}

		time::point wait_start = time::get_now();
		float seconds_waited = 0.0f;
		while (!data->is_all_finished() && seconds_waited < args.m_max_wait_seconds)
		{
			if (!try_cleanup_a_task()) // might as well do some cleanup
			{

			}

			if (args.m_wait_func)
				args.m_wait_func();

			seconds_waited = time::get_ms_between<float>(time::get_now(), wait_start) * 0.001f;
		}
		if (args.mp_out_seconds_waited != nullptr)
		{
			(*args.mp_out_seconds_waited) = seconds_waited;
		}
	}

	void async_manager::add_child(const task_handle& parent, const task_handle& child)
	{
		if (!parent.is_valid() || !child.is_valid() || (parent == child))
		{
			return;
		}

		++parent.mp_data->m_num_children_unfinished;
		parent.mp_data->m_children.push_back(child.mp_data);
		child.mp_data->m_parent = parent.mp_data;
	}

	bool async_manager::has_parent(const task_handle& handle)
	{
		if (!handle.is_valid())
		{
			return false;
		}

		return (handle.mp_data->m_parent != nullptr);
	}

	void async_manager::set_requeue_condition(const task_handle& handle, const function<bool()>& condition_func)
	{
		if (!handle.is_valid())
		{
			return;
		}

		handle.mp_data->m_requeue_condition = condition_func;
	}

	async_manager::work_queue& async_manager::get_global_queue()
	{
		return m_global_queue;
	}

	async_manager::work_queue& async_manager::get_global_cleanup_queue()
	{
		return m_global_cleanup_queue;
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
	constexpr uint64 get_static_num_bytes()
	{
		return sizeof(async_manager::get_instance());
	}

	void initialize(const init_args& args)
	{
		return async_manager::get_instance().initialize(args);
	}

	task_handle create_task(const task_args& args)
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

	void add_child(const task_handle& parent, const task_handle& child)
	{
		async_manager::get_instance().add_child(parent, child);
	}

	bool has_parent(const task_handle& handle)
	{
		return async_manager::get_instance().has_parent(handle);
	}

	void set_requeue_condition(const task_handle& handle, const function<bool()>& condition_func)
	{
		async_manager::get_instance().set_requeue_condition(handle, condition_func);
	}

	void shutdown()
	{
		async_manager::get_instance().shutdown();
	}
#pragma endregion
}