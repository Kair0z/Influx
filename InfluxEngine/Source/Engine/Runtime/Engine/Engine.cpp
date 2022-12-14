#include "pch.h"
#include "Engine.h"

#include "Engine/Runtime/Logger/Logger.h"
#include "Engine/Runtime/Profiling/Profiler.h"
#include "Engine/Runtime/Memory/Memory.h"

namespace Influx
{
	void Engine::Initialize()
	{
		Global::mp_logger = mp_logger = new Logger();
		Global::mp_profiler = mp_profiler = new Profiler();
		
		mp_memory = new Memory();
	}
}

