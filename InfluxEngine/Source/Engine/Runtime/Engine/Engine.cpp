#include "pch.h"
#include "Engine.h"

#include "Engine/Runtime/Logger/Logger.h"
#include "Engine/Runtime/Profiling/Profiler.h"
#include "Engine/Runtime/Memory/Memory.h"

namespace Influx
{
	void Engine::Initialize()
	{
		mp_memory = new Memory();
		mp_logger = new Logger();
		mp_profiler = new Profiler();
	}

	void Engine::Cleanup()
	{
		SafeDelete(mp_memory);
		SafeDelete(mp_logger);
		SafeDelete(mp_profiler);
	}
}

