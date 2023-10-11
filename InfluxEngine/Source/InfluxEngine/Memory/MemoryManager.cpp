#include "engine_pch.h"
#include "InfluxEngine/Memory/MemoryManager.h"

namespace influx
{
	void* MemoryManager::Allocate(uint64 dimension)
	{
		return platform::Allocate(dimension);
	}

	void MemoryManager::Free(void* pointer)
	{
		return platform::Free(pointer);
	}
}