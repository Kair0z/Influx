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
#include "core/wait.h"

namespace influx::async
{
	template <typename _t = char>
	using result = influx::result<_t, debug_name>;
	enum class e_log { info, warning, error, count };
	typedef void (*log_function)(e_log, const char*);

	// task handle
	constexpr static uint64 k_task_invalid_idx = (uint64)-1;

	enum class e_task_state : uint8
	{
		allocated,		// the task is pre-allocated in memory, but won't be ran (yet)
		queued,			// the task is waiting to be executed
		running,		// the task is running
		finished,		// the task has completed
		max,
		invalid = max
	};

	template <typename _t>
	struct task_create_args final
	{
		typedef void (*entrypoint_func)(_t*);

		// debug_name		m_name = "";
		entrypoint_func		m_entrypoint = {};
		_t*					m_data;

		task_create_args() = default;
		task_create_args(entrypoint_func entry, _t* data)
			: m_entrypoint{ entry }, m_data{ data } { }
	};

	struct task_stats final
	{
		float m_seconds_queued = 0.0f;
		float m_seconds_running = 0.0f;
		float m_seconds_total = 0.0f;
	};

	class task_handle final
	{
		uint64 m_task_data_idx = k_task_invalid_idx;
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
		friend class async_manager;
		friend struct task_data;
		task_handle(uint64 task_idx);

		// queries the manager to find the data associated with this handle
		result<struct task_data*> find_task_data() const;
	};

	namespace detail
	{
		typedef void (*entrypoint_func_internal)(void*);
		struct task_create_args_internal final
		{
			entrypoint_func_internal m_entrypoint;
			void* m_data;
		};

		template <typename _t>
		inline task_create_args_internal convert_args(const task_create_args<_t>& typed_args);

		INFLUX_ASYNC_API
		result<task_handle> create_task(const task_create_args_internal& args);
	}

	// defines & constants
#pragma region constants
#define INFLUX_ASYNC_OMIT_STATS 1
#if INFLUX_ASYNC_OMIT_STATS
	static constexpr uint64 k_task_data_stats_bytesize = 0u;
#else
	static constexpr uint64 k_num_timepoints_per_task_data = 4u;
	static constexpr uint64 k_task_data_stats_bytesize = sizeof(task_stats) + (k_num_timepoints_per_task_data * 8u);
#endif
	static constexpr uint64 k_task_data_bytesize = 72u + k_task_data_stats_bytesize;
	static constexpr uint64 k_taskbuffer_capacity_bytes = 32u * 4096u * 4096u;
	static constexpr uint64 k_max_num_tasks_in_flight = k_taskbuffer_capacity_bytes / k_task_data_bytesize;
	static constexpr uint64 k_max_num_threads = 16u;
#pragma endregion

	struct init_args final
	{
		// optional log callback
		log_function m_log_callback = nullptr;

		// max num threads occupied to do work
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API
	result<> initialize(const init_args& args);

	template <typename _t>
	result<task_handle> create_task(task_create_args<_t>&& args = {}) {
		return detail::create_task(detail::convert_args<_t>(args));
	}

	INFLUX_ASYNC_API
	result<> dispatch(const task_handle& handle);

	INFLUX_ASYNC_API 
	result<> wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API 
	result<> wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API
	result<> wait_for_all(const wait_args& args = {});

	INFLUX_ASYNC_API 
	result<> shutdown();

	inline 
	result<> dispatch(const vector<task_handle>& handles)
	{
		for (const task_handle& handle : handles)
		{
			dispatch(handle);
		}
		return{};
	}

	template <typename _t>
	inline result<> dispatch(const task_create_args<_t>& args)
	{
		auto create_result = create_task<_t>(args);
		
		if (create_result.is_success() == false)
			return result<>::make_error("create_task() failed!");

		return dispatch(create_result.get());
	}

#if 0
	inline
	result<> dispatch(const function<void()>& func)
	{
		return dispatch({ "", func });
	}

	inline 
	result<vector<task_handle>> dispatch_for(uint64 range, const function<void(uint64 i)>& func)
	{
		using result_type = result<vector<task_handle>>;
		if (func == nullptr)
			return result_type::make_error("func is null, not queueing tasks");

		if (range >= k_max_num_tasks_in_flight)
			return result_type::make_error("cannot allocate a range of tasks larger than the max allocatable!");

		vector<task_handle> handles{}; handles.reserve(range);
		for (uint64 i = 0u; i < range; ++i)
		{
			// make a sub-function that passes the index as parameter
			auto sub_func = [i, func]() { func(i); };

			// create the task
			auto created_result = create_task({ "foreach_" + to_string(i) }, sub_func);
			if (created_result.is_success() == false)
			{
				return result_type::make_error("failed allocating task to dispatch!");
			}

			// collect the handle
			handles.push_back(created_result.get());
		}

		// dispatch the handles
		auto dispatch_res = dispatch(handles);
		if (dispatch_res.is_success() == false)
		{
			return result_type::make_error("failed dispatching the created tasks");
		}

		return handles;
	}
#endif

	// iterates over a const vector<_t>&
	template <typename _t>
	inline 
	result<vector<task_handle>> dispatch_for(const vector<_t>& vector, const function<void(const _t& i)>& func)
	{
		auto sub_func = [vector, func](uint64 i) { func(vector[i]); };
		return dispatch_for(vector.size(), sub_func);
	}
	
#if _DEBUG
	// DONT TOUCH THIS, IN DEBUG BUILDS GIVES US A PEAK TO PRIVATE GLOBAL STATE
	static class async_manager* gp_global_manager_state = nullptr;
#endif

	extern constexpr uint64 INFLUX_ASYNC_API get_static_num_bytes();

	namespace detail
	{
		template <typename _t>
		inline task_create_args_internal convert_args(const task_create_args<_t>& typed_args)
		{
			return task_create_args_internal{
				.m_entrypoint	= reinterpret_cast<void(*)(void*)>( typed_args.m_entrypoint ),
				.m_data			= static_cast<void*>( typed_args.m_data )
			};
		}
	}
}