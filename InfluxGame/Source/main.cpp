
/*
*	[INFLUX GAME]
*	
*
*/

#include "influx_application.h"
#pragma comment(lib, "InfluxApplication.lib")

#include <iostream>

int main(int argc, char** argv)
{
	influx::application::run_args arguments{};
	arguments.m_commandlet = false;
	arguments.m_vsync = false;
	arguments.m_name = "Influx Game";

	influx::application::run(arguments);

	influx::application::quit();
}