#ifndef __ENGINE_OBJECT_H_
#define __ENGINE_OBJECT_H_

#pragma once

namespace Influx
{
	class IObject
	{
	public:
		void* operator new(uint64 size);
		void* operator new[](uint64 size);
		void operator delete(void* pointer);
		void operator delete[](void* pointer);
	};
}

#endif