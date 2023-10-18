
/*
*	[INFLUX GAME]
*	
*
*/

#include "influx_application.h"
#pragma comment(lib, "InfluxApplication.lib")

int main(int argc, char** argv)
{
	influx::application::run_args arguments{};
	arguments.m_name = "Influx Game";

	influx::application::run(arguments);
}