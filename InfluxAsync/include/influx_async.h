#pragma once

#if _DLL
	#define INFLUX_ASYNC_API __declspec(dllexport)
#else
	#define INFLUX_ASYNC_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/function.h"

namespace influx::async
{
	using task_id = uint64;
	constexpr static task_id k_task_id_invalid = (uint64)-1;
	constexpr static uint64 k_max_num_tasks_in_flight = 4096u;

	extern constexpr uint64 INFLUX_ASYNC_API get_static_num_bytes();

	struct INFLUX_ASYNC_API wait_args final
	{
		float m_max_wait_seconds = -1.0f;
		float* mp_out_seconds_waited = nullptr;
	};

	enum class e_task_state
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

		bool operator==(const task_handle& other) const;
		bool operator!=(const task_handle& other) const;
		bool is_equal(const task_handle& other) const;
		e_task_state get_state() const;
		task_stats get_stats() const;

		void wait(const wait_args& args = {}) const;
		void dispatch() const;

	private:
		task_handle() = default;
		task_handle(task_id id);
		
		friend class async_manager;
		friend struct task_data;
	};

	struct task_args final
	{
		task_args() = default;
		task_args(const function<void()>& func) : m_func_execute{ func } { }
		function<void()> m_func_execute = {};
	};

	struct INFLUX_ASYNC_API init_args final
	{
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API void initialize(const init_args& args);

	INFLUX_ASYNC_API task_handle create_task(const task_args& args = {});

	INFLUX_ASYNC_API void dispatch(const task_handle& handle);

	INFLUX_ASYNC_API void dispatch(const vector<task_handle>& handles);

	INFLUX_ASYNC_API void wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API void wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API void shutdown();

#if _DEBUG
	static class async_manager* gp_global_manager_state = nullptr;
#endif
}