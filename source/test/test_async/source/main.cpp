#include "core/basetypes.h"
#include "core/time.h"
#include "core/math/math.h"
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

	uint32 num_iterations = 1024 * 4;

	double sync_time_in_seconds{};
	uint32 sync_sum = 0u;
	{
		time::point before = time::get_now();
		for (uint32 i = 0u; i < num_iterations; ++i)
		{
			sync_sum += (i + 1) * 2;
		}
		sync_time_in_seconds = time::get_ms_between<double>(time::get_now(), before) * 0.001;
	}

	double async_time_in_seconds{};
	uint32 async_sum = 0u;
	{
		time::point before = time::get_now();
		std::mutex mutex{};
		auto tasks = async::dispatch_for(num_iterations, [&mutex, &async_sum](uint64 i)
		{
			mutex.lock();
			async_sum += (i + 1) * 2;
			mutex.unlock();
		});
		async::wait_for(tasks.get());
		async_time_in_seconds = time::get_ms_between<double>(time::get_now(), before) * 0.001;
	}

	assert(async_sum == sync_sum);
	print_sync_vs_async_comparison(sync_time_in_seconds, async_time_in_seconds);
}

int main()
{
	static uint32 counter = 0u;

	async::init_args init_args{};
	init_args.m_num_workers = 4u;
	init_args.m_log_callback = nullptr;
	async::initialize(init_args).get();

	test_sums();

	async::shutdown();
}