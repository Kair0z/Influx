#include "engine_pch.h"
#include "InfluxEngine/Memory/MemoryManager.h"

namespace Influx
{
	void* MemoryManager::Allocate(uint64 size)
	{
		return Platform::Allocate(size);
	}

	void MemoryManager::Free(void* pointer)
	{
		return Platform::Free(pointer);
	}
}