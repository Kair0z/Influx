#include "core/basetypes.h"
#include "core/time.h"
#include "core/math/math.h"
#include "core/scope.h"
#include "influx_async.h"

#include <iostream>

using namespace influx;

void print_sync_vs_async_comparison(double sync_seconds, double async_seconds)
{
	sync_seconds = math::maximum(sync_seconds, 0.0000001);
	async_seconds = math::maximum(async_seconds, 0.0000001);

	std::cout << "- sync time (s): " << sync_seconds << "\n";
	std::cout << "- async time (s): " << async_seconds << "\n";
	if (sync_seconds > async_seconds)
	{
		std::cout << "- async runs " << (async_seconds / sync_seconds) * 100 << "% faster...\n";
	}
	else
	{
		std::cout << "- async runs " << (async_seconds / sync_seconds) * 100 << "% slower...\n";
	}
}

void test_sums()
{
	std::cout << "-- testing sums... \n";

	struct task_data final
	{
		uint32 i = 0u;
	};

	task_data* data = new task_data();
	auto task_func = [](task_data* data) {
		// scoped_event add{ "add" };
			data->i += 1u;
			data->i /= 4u;
			data->i = std::sqrt(data->i);
		};

	static constexpr uint32 k_num = 64 * 10u;
	async::task_handle tasks[k_num];
	for (uint32 i = 0u; i < k_num; ++i)
	{
		tasks[i] = async::create_task<task_data>({ task_func, data }).get();
	}

	double async_time_in_seconds{};
	uint32 async_sum = 0u;
	{
		time::point before = time::get_now();
		std::mutex mutex{};
		for (uint32 i = 0u; i < k_num; ++i)
		{
			async::dispatch(tasks[i]);
		}
		async::wait_for_all();
		async_time_in_seconds = time::get_ms_between<double>(time::get_now(), before) * 0.001;
	}

	double sync_time_in_seconds{};
	uint32 sync_sum = 0u;
	{
		time::point before = time::get_now();
		for (uint32 i = 0u; i < k_num; ++i)
		{
			task_func(data);
		}
		sync_time_in_seconds = time::get_ms_between<double>(time::get_now(), before) * 0.001;
	}

	// assert(async_sum == sync_sum);
	print_sync_vs_async_comparison(sync_time_in_seconds, async_time_in_seconds);
}

int main()
{
	static uint32 counter = 0u;

	async::init_args init_args{};
	init_args.m_num_workers = 8u;
	init_args.m_log_callback = nullptr;

	async::initialize(init_args).get();
	test_sums();
	async::shutdown();
}