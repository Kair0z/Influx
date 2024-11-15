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

namespace influx::async
{
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
		float m_seconds_pending = 0.0f;
		float m_seconds_running = 0.0f;
		float m_seconds_total = 0.0f;
	};

	struct task_create_args final
	{
		task_create_args() = default;
		task_create_args(const string& name, const function<void()>& func)
			: m_name{ name }, m_func_execute{ func } { }

		function<void()> m_func_execute = {};
		string m_name = "task";
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
		INFLUX_ASYNC_API e_task_state get_state() const;
		INFLUX_ASYNC_API task_stats get_stats() const;
		
		// copy & move constructable
		INFLUX_ASYNC_API task_handle(const task_handle& other);
		INFLUX_ASYNC_API task_handle(task_handle&& other) noexcept;
		INFLUX_ASYNC_API task_handle& operator=(const task_handle& other);
		INFLUX_ASYNC_API task_handle& operator=(task_handle&& other) noexcept;

	private:
		size_t m_task_data_idx = -1;

		friend class async_manager;
		friend struct task_data;
		task_handle(size_t task_idx = (size_t)-1);
		task_data* get_task_data() const;
	};

	// global API
	struct INFLUX_ASYNC_API init_args final
	{
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API void initialize(const init_args& args);

	INFLUX_ASYNC_API task_handle create_task(const task_create_args& args = {});
	
	inline task_handle create_task(const string& name, const function<void()>& func)
	{
		return create_task(task_create_args{ name, func });
	}

	INFLUX_ASYNC_API void dispatch(const task_handle& handle);

	inline void dispatch(const vector<task_handle>& handles)
	{
		for (const task_handle& handle : handles)
		{
			dispatch(handle);
		}
	}

	inline task_handle dispatch(const task_create_args& args)
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

			handles.push_back(create_task({ "foreach_" + to_string(i) }, sub_func));
		}

		dispatch(handles);
		return handles;
	}

	// iterates over a const vector<_t>&
	template <typename _t>
	inline vector<task_handle> dispatch_for(const vector<_t>& vector, const function<void(const _t& i)>& func)
	{
		auto sub_func = [vector, func](uint64 i)
		{
			func(vector[i]);
		};

		return dispatch_for(vector.size(), sub_func);
	}

	INFLUX_ASYNC_API void wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API void wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API void wait_for_all(const wait_args& args = {});

	INFLUX_ASYNC_API void shutdown();
	
#if _DEBUG
	// DONT TOUCH THIS, IN DEBUG BUILDS GIVES US A PEAK TO PRIVATE GLOBAL STATE
	static class async_manager* gp_global_manager_state = nullptr;
#endif
}