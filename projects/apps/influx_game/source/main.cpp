
#include "influx_application.h"

int main(int argc, char** argv)
{
	using namespace influx;

	application::run_args arguments{};
	arguments.m_commandlet = false;
	arguments.m_single_threaded = false;
	arguments.m_vsync = false;
	arguments.m_enable_editor = true;
	arguments.m_name = "Influx Game";
	arguments.m_window_clear_colour = math::float4{ 0.2f, 0.2f, 0.2f, 1.0f };
	arguments.m_staged = true;

	application::run(arguments);
}