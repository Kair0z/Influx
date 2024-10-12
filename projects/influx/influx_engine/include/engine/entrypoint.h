#pragma once
#ifndef INFLUX_ENGINE_MAIN
#define INFLUX_ENGINE_MAIN

// defined in influx_engine
namespace influx::engine::detail
{
	void run_engine();
}

int main()
{
	influx::engine::detail::run_engine();
	return 0;
}
#endif