#include "core/basetypes.h"
#include "influx_async.h"

#include <iostream>

using namespace influx;

int main()
{
	async::task_handle task = async::create_task("task", []()
		{
			std::cout << "execute!\n";
		});

	task.dispatch();
}