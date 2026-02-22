#include "influx_app.h"

int main(int argc, char* argv[])
{
	using namespace influx::app;

	app main_app{};
	main_app.set_enabled(app::component_flags::console);
	main_app.set_enabled(app::component_flags::window);
	main_app.set_enabled(app::component_flags::plugins);
	main_app.set_enabled(app::component_flags::editor);
	main_app.run({.m_argc = argc, .m_argv = argv, .m_run_async = false }).get();
}