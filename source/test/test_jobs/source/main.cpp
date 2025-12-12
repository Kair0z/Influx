#include "core/basetypes.h"
#include "core/time.h"
#include "core/math/math.h"
#include "influx_jobs.h"

#include <iostream>

using namespace influx;

void print_cmp(double sync_seconds, double async_seconds)
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

	uint32 num_iterations = 1024u * 4u;

	jobs::job_queue queue{};

	double async_time_in_seconds{};
	uint32 async_sum = 0u;
	{
		time::point before = time::get_now();
		
		// create the jobs
		vector<jobs::job_id> jobs{}; jobs.resize(num_iterations);
		for (uint32 i = 0u; i < num_iterations; ++i)
		{
			jobs[i] = queue.create_job({}).get();
		}
		// link all jobs
		for (uint32 i = 0u; i < num_iterations; ++i)
		{
			const bool has_next_job = i + 1 < num_iterations;
			if (has_next_job)
			{
				queue.set_dependency(jobs[i], jobs[i + 1]).get();
			}
		}
		// submit the jobs
		for (uint32 i = 0u; i < num_iterations; ++i)
		{
			queue.queue_job(jobs[i]).get();
		}

		async_time_in_seconds = time::get_ms_between<double>(time::get_now(), before) * 0.001;
	}

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

	// assert(async_sum == sync_sum);
	print_sync_vs_async_comparison(sync_time_in_seconds, async_time_in_seconds);
}

int main()
{
}