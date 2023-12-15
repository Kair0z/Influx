#pragma once
#include "influx_async.h"

#include "core/singleton/Singleton.h"
#include "core/container/queue.h"
#include "core/container/vector.h"
#include "core/container/list.h"
#include "core/container/pool.h"
#include "core/container/ringBuffer.h"
#include "Core/Math/Math.h"
#include "Core/Time.h"
#include <thread>
#include <mutex>

namespace influx::async
{
	struct task_data final
	{
		task_data() = default;
		task_data(const task_create_args& args)
			: m_args{ args }
		{
		}

		void reset(e_task_state state)
		{
			m_time_allocated = m_time_started = m_time_finished = time::get_now();
			m_state = state;
			m_stats = task_stats{};
		}

		inline bool is_finished() const
		{
			return m_state == e_task_state::finished;
		}

		task_handle m_handle{};
		e_task_state m_state{};
		task_create_args m_args{};
		task_stats m_stats{};

		time::point m_time_allocated = time::get_now();
		time::point m_time_started = time::get_now();
		time::point m_time_finished = time::get_now();
	};

	class async_manager final 
		: public singleton<async_manager>
	{
	public:
		using task_pool = pool<task_data, k_max_num_tasks_in_flight>;
		using task_queue = ringbuffer<task_data*, k_max_num_tasks_in_flight>;

		class worker_state final
		{
		public:
			worker_state() = default;
			worker_state(const uint64 id) : m_worker_id{ id }{}
			const uint64 m_worker_id = 0u;
		};

	public:
		void initialize(const init_args& args);
		void shutdown();

		task_handle create_task(const task_create_args& args = {});
		std::vector<task_handle> create_tasks(const std::vector<task_create_args>& args);

		void dispatch(const task_handle& handle);
		void dispatch(const vector<task_handle>& handles);
		void dispatch(task_data* data);
		void dispatch(const vector<task_data*>& datas);

		void wait_for(const task_handle& handle, const wait_args& args = {});
		void wait_for(const vector<task_handle>& handles, const wait_args& args = {});

		task_queue& get_global_queue();
		task_queue& get_global_cleanup_queue();

		task_data* get_task_data_from_handle(const task_handle& handle);

	private:
		bool m_is_initialized = false;
		vector<worker_state> m_worker_threads{};

		task_pool m_taskpool{};
		task_queue m_global_queue{};
		task_queue m_global_cleanup_queue{};

		// single pop, false if fail
		bool try_cleanup_a_task();
		// single pop, false if fail
		bool try_process_a_task();
		void cleanup_task(const task_handle& handle);
		void process_task(task_data* data);
		void cleanup_task(task_data* data);

		static void worker_thread_method(worker_state& state);
	};
}


