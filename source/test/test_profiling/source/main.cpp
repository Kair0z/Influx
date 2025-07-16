
#include "core/time.h"
#include "core/basetypes.h"
#include "core/commandline.h"
#include <iostream>

using namespace influx;

void print_line(const char* line)
{
	std::cout << line << "\n";
}

void run_profile(uint32 num_runs)
{
	for (uint32 i = 0u; i < num_runs; ++i)
	{

	}
}

int main(int argc, char* argv[])
{
	commandline arguments{ argc, argv };

	uint32 num_runs = 0u;

	// get all '-'arguments
	for (const commandline::argument* arg
		: arguments.get_arguments(commandline::e_common_prefix::lineline))
	{
		string as_string = string(arg->m_cstring);
		if (regex::has_match(as_string, "num"))
		{
			num_runs = 10;
			print_line(as_string.c_str());
		}
	}

	run_profile(num_runs);
}