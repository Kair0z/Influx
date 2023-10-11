#ifndef __ENGINE_MEMORYMANAGER_H_
#define __ENGINE_MEMORYMANAGER_H_

#pragma once

namespace influx
{
	// https://developer.ibm.com/tutorials/au-memorymanager/

	class MemoryManager final
	{
	public:
		void* Allocate(uint64 dimension);
		void Free(void* pointer);
	};
}

#endif