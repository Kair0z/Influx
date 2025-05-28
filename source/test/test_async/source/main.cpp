#include "core/basetypes.h"
#include "influx_async.h"

#include <iostream>

using namespace influx;

int main()
{
	async::init_args init_args{};
	init_args.m_num_workers = 4u;
	init_args.m_log_callback = nullptr;
	async::initialize(init_args).get();

	async::task_handle task = async::create_task("task", []()
	{
		std::cout << "execute!\n";

	}).get();

	task.dispatch();
	task.wait();

	async::shutdown();
}