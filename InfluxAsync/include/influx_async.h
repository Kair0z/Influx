#pragma once

#if _DLL
	#define INFLUX_ASYNC_API __declspec(dllexport)
#else
	#define INFLUX_ASYNC_API __declspec(dllimport)
#endif

#include "Core/basetypes.h"

namespace influx::async
{
	using task_id = uint64;

	class INFLUX_ASYNC_API task_handle final
	{
		task_id m_id = 0u;
	};

	struct INFLUX_ASYNC_API init_args final
	{
		int m_num_workers = 2u;
	};

	INFLUX_ASYNC_API void initialize(const init_args& args);

	INFLUX_ASYNC_API task_handle queue_task();

	INFLUX_ASYNC_API void wait_for(const task_handle& handle);

	INFLUX_ASYNC_API void shutdown();
}