#pragma once

#if _DLL
	#define INFLUX_ASYNC_API __declspec(dllexport)
#else
	#define INFLUX_ASYNC_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/function.h"
#include "core/string.h"
#include "core/result.h"

namespace influx::async
{
	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	constexpr static uint64 k_max_num_tasks_in_flight = 4096u;
	extern constexpr uint64 INFLUX_ASYNC_API get_static_num_bytes();

	struct wait_args final
	{
		float m_max_ms = FLT_MAX;
		float* mp_out_ms = nullptr;
		function<void()> m_wait_func{};
	};

	enum class e_task_state : uint8
	{
		allocated,
		queued,
		running,
		finished,
		max,
		invalid = max
	};

	struct task_stats final
	{
		float m_seconds_queued = 0.0f;
		float m_seconds_running = 0.0f;
		float m_seconds_total = 0.0f;
	};

	struct task_create_args final
	{
		string m_name = "task";
		function<void()> m_func_execute = {};

		task_create_args() = default;
		task_create_args(const string& name, const function<void()>& func)
			: m_name{ name }, m_func_execute{ func } { }
	};

	class task_handle final
	{
	public:
		INFLUX_ASYNC_API bool is_valid() const;

		INFLUX_ASYNC_API void dispatch() const;
		INFLUX_ASYNC_API void wait(const wait_args& args = {}) const;
		INFLUX_ASYNC_API bool is_finished() const;

		INFLUX_ASYNC_API bool operator==(const task_handle& other) const;
		INFLUX_ASYNC_API bool operator!=(const task_handle& other) const;
		INFLUX_ASYNC_API bool is_equal(const task_handle& other) const;

		// returns state if is_valid() == true, else returns error result
		INFLUX_ASYNC_API result<e_task_state> get_state() const;

		// returns stats if is_valid() == true, else returns error result
		INFLUX_ASYNC_API result<task_stats> get_stats() const;
		
		// copy & move constructable
		INFLUX_ASYNC_API task_handle();
		INFLUX_ASYNC_API task_handle(const task_handle& other);
		INFLUX_ASYNC_API task_handle(task_handle&& other) noexcept;
		INFLUX_ASYNC_API task_handle& operator=(const task_handle& other);
		INFLUX_ASYNC_API task_handle& operator=(task_handle&& other) noexcept;

	private:
		uint64 m_task_data_idx = -1;

		friend class async_manager;
		friend struct task_data;
		task_handle(uint64 task_idx);

		// queries the manager to find the data associated with this handle
		result<struct task_data*> find_task_data() const;
	};

	enum class e_log { info, warning, error, count };
	typedef void (*log_function)(e_log, const char*);

	struct init_args final
	{
		// optional log callback
		log_function m_log_callback = nullptr;

		// max num threads occupied to do work
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API
	result<> initialize(const init_args& args);

	INFLUX_ASYNC_API
	result<task_handle> create_task(const task_create_args& args = {});
	
	inline
	result<task_handle> create_task(const string& name, const function<void()>& func)
	{
		task_create_args new_args{};
		new_args.m_func_execute = func;
		new_args.m_name = name;
		return create_task(new_args);
	}

	INFLUX_ASYNC_API 
	result<> dispatch(const task_handle& handle);

	inline 
	result<> dispatch(const vector<task_handle>& handles)
	{
		for (const task_handle& handle : handles)
		{
			dispatch(handle);
		}
		return{};
	}

	inline 
	result<> dispatch(const task_create_args& args)
	{
		auto create_result = create_task(args);
		
		if (create_result.is_success() == false)
			return result<>::make_error("failed creating task with these args");

		return dispatch(create_result.get());
	}

	inline 
	result<> dispatch(const function<void()>& func)
	{
		return dispatch({ "", func });
	}

	inline 
	result<vector<task_handle>> dispatch_for(uint64 range, const function<void(uint64 i)>& func)
	{
		if (func == nullptr)
			return {};

		vector<task_handle> handles{};
		handles.reserve(range);
		for (uint64 i = 0u; i < range; ++i)
		{
			auto sub_func = [i, func]()
			{
				func(i);
			};

			handles.push_back(create_task({ "foreach_" + to_string(i) }, sub_func).get());
		}

		dispatch(handles);
		return handles;
	}

	// iterates over a const vector<_t>&
	template <typename _t>
	inline 
	result<vector<task_handle>> dispatch_for(const vector<_t>& vector, const function<void(const _t& i)>& func)
	{
		auto sub_func = [vector, func](uint64 i)
		{
			func(vector[i]);
		};

		return dispatch_for(vector.size(), sub_func);
	}

	INFLUX_ASYNC_API result<> wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API result<> wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API result<> wait_for_all(const wait_args& args = {});

	INFLUX_ASYNC_API result<> shutdown();
	
#if _DEBUG
	// DONT TOUCH THIS, IN DEBUG BUILDS GIVES US A PEAK TO PRIVATE GLOBAL STATE
	static class async_manager* gp_global_manager_state = nullptr;
#endif
}