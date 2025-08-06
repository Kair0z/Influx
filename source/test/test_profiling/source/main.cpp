
#include "core/time.h"
#include "core/basetypes.h"
#include "core/commandline.h"
#include <iostream>

using namespace influx;

void very_slow()
{
	int* a = new int();
}

void print_line(const char* line)
{
	std::cout << line << "\n";
}

template <typename _func>
void run_profile(uint32 num_runs, _func&& func)
{
	time::point before = time::get_now();
	for (uint32 i = 0u; i < num_runs; ++i)
	{
		func();
	}
	time::point after = time::get_now();

	std::cout << "x" << std::to_string(num_runs) << " | ";
	std::cout << std::to_string( time::get_ns_between<float>(after, before) ) << " ns\n";
}

int main(int argc, char* argv[])
{
	commandline arguments{ argc, argv };
	uint32 num_runs = 0u;

	// get all '--'arguments
	for (const commandline::argument* arg : arguments.get_arguments(commandline::e_common_prefix::lineline))
	{
		string as_string = string(arg->m_cstring);
		if (regex::has_match(as_string, "num"))
		{
			num_runs += std::stoi(as_string.substr(_countof("--num=") - 1));
		}
	}

	std::cout << "PROFILE =================\n";
	for (uint32 i = 0u; i < 10u; ++i)
	{
		run_profile(num_runs, []()
		{
			very_slow();
		});
	}
}