#pragma once

#if _DLL
	#define INFLUX_ASYNC_API __declspec(dllexport)
#else
	#define INFLUX_ASYNC_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/function.h"
#include "Core/String.h"

#include <thread>

namespace influx::async
{
	using task_id = uint64;
	constexpr static task_id k_task_id_invalid = (uint64)-1;
	constexpr static uint64 k_max_num_tasks_in_flight = 4096u;
	extern constexpr uint64 INFLUX_ASYNC_API get_static_num_bytes();

	struct wait_args final
	{
		float m_max_wait_seconds = FLT_MAX;
		float* mp_out_seconds_waited = nullptr;
		function<void()> m_wait_func{};
	};

	enum class e_task_state : uint8
	{
		pending,
		running,
		finished,
		max,
		invalid = max
	};

	struct task_stats final
	{
		float m_seconds_pending = 0.0f;
		float m_seconds_running = 0.0f;
		float m_seconds_total = 0.0f;
	};

	/*
	* task_handle:
	*/
	class INFLUX_ASYNC_API task_handle final
	{
		task_id m_id = 0u;
		struct task_data* mp_data = nullptr;

	public:
		bool is_valid() const;

		void dispatch() const;
		void wait(const wait_args& args = {}) const;

		bool is_finished_self() const;
		bool are_children_finished() const;
		bool is_finished_all() const;

		bool has_parent() const;
		void add_child(const task_handle& child);

		bool operator==(const task_handle& other) const;
		bool operator!=(const task_handle& other) const;
		bool is_equal(const task_handle& other) const;
		e_task_state get_state() const;
		task_stats get_stats() const;
		
		void set_requeue_condition(const function<bool()>& condition_func);

	private:
		task_handle() = default;
		task_handle(task_id id);
		
		friend class async_manager;
		friend struct task_data;
	};

	struct task_args final
	{
		task_args() = default;
		task_args(const string& name, const function<void()>& func) 
			: m_name{ name }, m_func_execute { func } { }
		
		function<void()> m_func_execute = {};
		string m_name = "task";
	};


	struct INFLUX_ASYNC_API init_args final
	{
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API void initialize(const init_args& args);

	INFLUX_ASYNC_API task_handle create_task(const task_args& args = {});
	
	inline task_handle create_task(const string& name, const function<void()>& func)
	{
		return create_task(task_args{ name, func });
	}

	INFLUX_ASYNC_API void dispatch(const task_handle& handle);

	INFLUX_ASYNC_API void dispatch(const vector<task_handle>& handles);

	inline task_handle dispatch(const task_args& args)
	{
		task_handle handle = create_task(args);
		dispatch(handle);
		return handle;
	}

	inline task_handle dispatch(const function<void()>& func)
	{
		return dispatch({ "", func });
	}

	inline vector<task_handle> dispatch_for(uint64 range, const function<void(uint64 i)>& func)
	{
		if (func != nullptr)
			return {};

		vector<task_handle> handles{};
		handles.reserve(range);
		for (uint64 i = 0u; i < range; ++i)
		{
			handles.push_back(create_task({ "foreach_" + to_string(i) }, [i, func]()
				{
					func(i);
				}));
		}

		for (task_handle& handle : handles)
		{
			handle.dispatch();
		}

		return handles;
	}


	INFLUX_ASYNC_API void wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API void wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API void add_child(const task_handle& parent, const task_handle& child);

	INFLUX_ASYNC_API bool has_parent(const task_handle& handle);

	INFLUX_ASYNC_API void set_requeue_condition(const task_handle& handle, const function<bool()>& condition_func);

	INFLUX_ASYNC_API void shutdown();

	inline int get_max_concurrency()
	{
		return std::thread::hardware_concurrency();
	}
	
#if _DEBUG
	static class async_manager* gp_global_manager_state = nullptr;
#endif
}