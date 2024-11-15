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
		void initialize(const init_args& args);
		void shutdown();
		bool is_shutdown();

		task_handle create_task(const task_create_args& args = {});
		std::vector<task_handle> create_tasks(const std::vector<task_create_args>& args);

		void dispatch(const task_handle& handle);
		void dispatch(const vector<task_handle>& handles);
		void dispatch(task_data* data);
		void dispatch(const vector<task_data*>& datas);

		void wait_for(const task_handle& handle, const wait_args& args = {});
		void wait_for(const vector<task_handle>& handles, const wait_args& args = {});
		void wait_for_all(const wait_args& args = {});

		task_queue& get_global_queue();
		task_queue& get_global_cleanup_queue();

		task_data* get_task_from_handle(const task_handle& handle);

		uint64 get_num_queued() const;
		uint64 get_num_processing() const;
		uint64 get_num_toclean() const;
		
		// true if we have tasks queued || processing
		bool has_unfinished_work() const; 
		
	private:
		bool m_is_initialized = false;
		vector<worker_state> m_worker_threads{};

		task_pool* mp_taskpool{};
		task_queue* mp_global_queue = nullptr;
		task_queue* mp_global_cleanup_queue = nullptr;

		bool grab_and_clean_a_task();
		bool grab_and_process_a_task();

		void do_cleanup_task(const task_handle& handle);
		void do_process_task(task_data* data);
		void do_cleanup_task(task_data* data);

		static void worker_thread_method(worker_state& state);

		std::atomic_uint64_t m_num_processing = 0u;
	};
}


