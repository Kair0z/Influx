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
	
	/*
	* task_handle:
	*/
	class INFLUX_ASYNC_API task_handle final
	{
		task_id m_id = 0u;

	public:
		bool is_valid() const;

		bool operator==(const task_handle& other) const;
		bool operator!=(const task_handle& other) const;
		bool is_equal(const task_handle& other) const;

	private:
		task_handle() = default;
		task_handle(task_id id);
		friend class async_manager;
	};

	struct INFLUX_ASYNC_API task_args final
	{
		function<void()> m_func_execute = {};
		function<void()> m_func_on_finish = {};
	};

	struct INFLUX_ASYNC_API init_args final
	{
		int m_num_workers = 2u;
	};

	struct INFLUX_ASYNC_API wait_args final
	{
		float m_max_wait_seconds = -1.0f;
	};

	INFLUX_ASYNC_API void initialize(const init_args& args);

	INFLUX_ASYNC_API task_handle create_task(const task_args& args = {});

	INFLUX_ASYNC_API void dispatch(const task_handle& handle);

	INFLUX_ASYNC_API void dispatch(const vector<task_handle>& handles);

	INFLUX_ASYNC_API void wait_for(const task_handle& handle, const wait_args& args = {});

	INFLUX_ASYNC_API void wait_for(const vector<task_handle>& handles, const wait_args& args = {});

	INFLUX_ASYNC_API void shutdown();
}