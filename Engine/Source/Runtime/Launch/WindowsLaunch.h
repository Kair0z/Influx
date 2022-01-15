#pragma once

#if PLATFORM_WINDOWS

#include "Runtime/Engine/Engine.h"

int main()
{
	Influx::Engine* engine = new Influx::Engine();
	engine->Run();

	delete engine;
}
#endif