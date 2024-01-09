
#if 1
#include "core/platform/windows/windows_window.h"
#else
#include "core/platform/null/null_window.h"
#endif

#include <iostream>

int main()
{
	using namespace influx;
	platform::window_handle window = platform::create_window({ { 640u, 480u }, "window" });

	std::cin.get();
	return 0u;
}