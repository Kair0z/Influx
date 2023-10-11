
/*
*	[INFLUX GAME]
*	
*
*/

#include <iostream>

#include "application/influx_application.h"
#pragma comment(lib, "InfluxApplication")

int main(int argc, char** argv)
{
	influx::application::run_args arguments{};
	arguments.m_name = "Influx Game";

	influx::application::run(arguments);

	if (std::cin.get())
	{
		influx::application::quit();
	}
}