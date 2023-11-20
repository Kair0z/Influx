
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
	arguments.m_single_threaded = false;
	arguments.m_vsync = true;
	arguments.m_enable_editor = false;
	arguments.m_name = "Influx Game";
	arguments.m_window_clear_colour = influx::math::float4{ 0.2f, 0.2f, 0.2f, 1.0f };

	influx::application::run(arguments);

	influx::application::quit();
}