#pragma once
#include "influx_async.h"

namespace influx::async
{
	class task_queue;

	class async_manager final
		: public singleton<async_manager>
	{
	public:
		using task_pool		= pool<task_data, k_max_num_tasks_in_flight>;

		class worker_state final
		{
		public:
			worker_state() = default;
			worker_state(const uint64 id) : m_worker_id{ id }{}
			const uint64 m_worker_id = 0u;

			std::thread m_thread_obj{};
		};

	public:
		result<> initialize(const init_args& args);
		result<> shutdown();
		bool is_shutdown();

		result<task_handle> create_task(const task_create_args& args = {});
		result<std::vector<task_handle>> create_tasks(const std::vector<task_create_args>& args);

		result<> dispatch(const task_handle& handle);
		result<> dispatch(const vector<task_handle>& handles);
		result<> dispatch(task_data* data);
		result<> dispatch(const vector<task_data*>& datas);

		result<> wait_for(const task_handle& handle, const wait_args& args = {});
		result<> wait_for(const vector<task_handle>& handles, const wait_args& args = {});
		result<> wait_for_all(const wait_args& args = {});

		task_queue& get_global_queue();
		task_queue& get_global_cleanup_queue();

		result<task_data*> get_task_from_handle(const task_handle& handle);

		result<uint64> get_num_queued() const;
		result<uint64> get_num_processing() const;
		result<uint64> get_num_toclean() const;
		
		// true if we have tasks queued || processing
		bool has_unfinished_work() const; 
		
	private:
		bool m_is_initialized = false;
		vector<worker_state> m_worker_threads{};

		task_pool* mp_taskpool{};
		task_queue* mp_global_queue = nullptr;
		task_queue* mp_global_cleanup_queue = nullptr;

		// tries to grab a task off  the cleanup queue, and 'recycle' it
		result<bool> try_grab_and_clean_a_task();

		// tries to a task off the given queue, and executes it
		result<bool> try_grab_and_process_a_task(task_queue& queue);

		result<> do_cleanup_task(const task_handle& handle);
		result<> do_process_task(task_data* data);
		result<> do_cleanup_task(task_data* data);

		static void worker_thread_method(worker_state& state);

		std::atomic_uint64_t m_num_processing = 0u;
	};
}


