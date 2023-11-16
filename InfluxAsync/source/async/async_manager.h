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
		task_data(const task_args& args)
			: m_args{ args }
		{
		}
		task_data(const task_data& other)
		{
			m_handle = other.m_handle;
			m_state = other.m_state;
			m_args = other.m_args;
			m_stats = other.m_stats;
			m_parent = other.m_parent;
			m_children = other.m_children;
			m_requeue_condition = other.m_requeue_condition;
			m_num_children_unfinished = other.m_num_children_unfinished.load();
		}
		task_data& operator=(const task_data& other)
		{
			m_handle = other.m_handle;
			m_state = other.m_state;
			m_args = other.m_args;
			m_stats = other.m_stats;
			m_parent = other.m_parent;
			m_children = other.m_children;
			m_requeue_condition = other.m_requeue_condition;
			m_num_children_unfinished = other.m_num_children_unfinished.load();
			return *this;
		}

		void reset(e_task_state state)
		{
			m_time_created = time::get_now();
			m_time_started = time::get_now();
			m_time_finished = time::get_now();
			m_state = state;
			m_stats = task_stats{};
			m_num_children_unfinished = static_cast<uint32>(m_children.size());
		}

		task_handle m_handle{};
		e_task_state m_state{};
		task_args m_args{};
		task_stats m_stats{};

		task_data* m_parent = nullptr;
		vector<task_data*> m_children{};
		function<bool()> m_requeue_condition = {};
		std::atomic_uint32_t m_num_children_unfinished = 0u;

		bool is_self_finished() const
		{
			return m_state == e_task_state::finished;
		}

		bool is_all_finished() const
		{
			return is_self_finished() && are_children_finished();
		}

		bool are_children_finished() const
		{
			return m_children.empty() || m_num_children_unfinished < 1u;
		}

		time::point m_time_created = time::get_now();
		time::point m_time_started = time::get_now();
		time::point m_time_finished = time::get_now();
	};

	class async_manager final 
		: public singleton<async_manager>
	{
	public:
		// the pool of task_data memory which we use as our allocator
		using task_pool = pool<task_data, k_max_num_tasks_in_flight>;

		// threadsafe ringbuffer for pushing & popping tasks
		struct work_queue final
		{
			work_queue() = default;
			ringbuffer<task_data*, k_max_num_tasks_in_flight> m_tasks{};
		};

		// state of a worker thread
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

		task_handle create_task(const task_args& args = {});

		void dispatch(const task_handle& handle);
		void dispatch(task_data* data);
		void wait_for(const task_handle& handle, const wait_args& args = {});
		void add_child(const task_handle& parent, const task_handle& child);
		bool has_parent(const task_handle& handle);
		void set_requeue_condition(const task_handle& handle, const function<bool()>& condition_func);

		work_queue& get_global_queue();
		work_queue& get_global_cleanup_queue();

		task_data* get_task_data_from_handle(const task_handle& handle);

	private:
		bool m_is_initialized = false;
		vector<std::thread> m_worker_threads{};
		vector<worker_state> m_worker_states{};

		task_pool m_taskpool{};
		vector<task_data*> mp_taskdatas{};

		// single pop, false if fail
		bool try_cleanup_a_task();
		// single pop, false if fail
		bool try_process_a_task();
		void cleanup_task(const task_handle& handle);
		void process_task(task_data* data);
		void cleanup_task(task_data* data);

		static void worker_thread_method(worker_state& state);

		work_queue m_global_queue{};
		work_queue m_global_cleanup_queue{};

		std::mutex m_cleanup_mutex{};
	};
}


