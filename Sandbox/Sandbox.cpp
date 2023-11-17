
#include <iostream>
#include "influx_async.h"
#pragma comment(lib, "InfluxAsync.lib")

uint64_t g_frame = 840u;

using namespace influx;

int main()
{
	async::init_args args{};
	args.m_num_workers = 1u;
	async::initialize(args);

	async::task_handle simulation_task = async::create_task("simulation", []()
	{
		std::cout << "Simulation! \n";
		--g_frame;
	});

	async::task_handle render_task = async::create_task("render", []()
	{
		std::cout << "Render! \n";
	});

	while (true)
	{
		
	}

	async::shutdown();
}