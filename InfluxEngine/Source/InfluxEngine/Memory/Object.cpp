#include "engine_pch.h"

#include "InfluxEngine/Memory/Object.h"
#include "InfluxEngine/Engine.h"

#include "InfluxEngine/Memory/MemoryManager.h"

namespace Influx
{
	void* IObject::operator new(uint64 size)
	{
		void* pointer = Engine::GetMemoryManager()->Allocate(size);
		return pointer;
	}

	void* IObject::operator new[](uint64 size)
	{
		return Engine::GetMemoryManager()->Allocate(size);
	}

	void IObject::operator delete(void* pointer)
	{
		Engine::GetMemoryManager()->Free(pointer);
	}

	void IObject::operator delete[](void* pointer)
	{
		Engine::GetMemoryManager()->Free(pointer);
	}
}

