#pragma once
#include "influx_async.h"
#include "core/singleton/Singleton.h"
#include "core/container/queue.h"
#include "core/container/vector.h"
#include "core/container/list.h"
#include "core/container/pool.h"

#include <thread>

namespace influx::async
{
	class async_manager final 
		: public singleton<async_manager>
	{
	public:
		enum class e_task_state
		{
			pending,
			running,
			finished,
			max
		};

		struct task_data final
		{
			task_data(const task_args& args)
				: m_args{args}
			{
			}

			task_handle m_handle{};
			e_task_state m_state{};
			task_args m_args{};
		};

		using task_pool = pool<task_data, 256u>;

		struct work_queue final
		{
			queue<task_handle> m_queued_tasks{};
		};

		class worker_state final
		{
		public:
			worker_state(const uint64 id) : m_worker_id{ id } {}
			const uint64 m_worker_id = 0u;
			work_queue m_my_queue{};
		};

	public:
		void initialize(const init_args& args);
		void shutdown();

		task_handle create_task(const task_args& args = {});

		void dispatch(const task_handle& handle);
		void wait_for(const task_handle& handle, const wait_args& args = {});

		work_queue& get_global_queue();
		task_data* get_task_data_from_handle(const task_handle& handle);

	private:
		bool m_is_initialized = false;
		vector<std::thread> m_worker_threads{};
		vector<worker_state> m_worker_states{};

		task_pool m_taskpool{};
		vector<task_data*> mp_taskdatas{};

		void cleanup_task(const task_handle& handle);
		void cleanup_task(task_data* data);

		static void worker_thread_method(worker_state& state);

		work_queue m_global_queue{};
	};
}


