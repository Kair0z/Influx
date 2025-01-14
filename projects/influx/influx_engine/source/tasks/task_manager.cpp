#include "engine_pch.h"
#include "task_manager.h"

// influx::async
#include "influx_async.h"

namespace influx::engine
{
	task_manager::task_manager()
	{
		async::init_args async_args{};
		async_args.m_num_workers = 1u;
		async::initialize(async_args);
	}

	task_manager::~task_manager()
	{
		async::shutdown();
	}
}