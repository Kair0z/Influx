#pragma once

#ifndef _CORE_ALLOCATORS_H_
#define _CORE_ALLOCATORS_H_

#include "BaseAllocator.h"
#include "Assert.h"

namespace Influx
{
	/* Simple Linear Allocator */
	// -> Memory can only be freed by clearing all allocations
	/* [INCOMPLETE] */
	class LinearAllocator final : public BaseAllocator
	{
	public:
		inline LinearAllocator(const size_t size, const void* pBase) : BaseAllocator(size, pBase){}
		inline virtual ~LinearAllocator() {}

		virtual void* Allocate(const size_t size) override {}
		virtual void Free(void*) override {}
		virtual void Clear() override {}
	};

	/* Simple Pool Allocator */
	// -> Memory is stored in chunks of objSize
	template <class Obj>
	class PoolAllocator final : public BaseAllocator
	{
		constexpr static size_t ObjectSize = sizeof(Obj);

	public:
		inline PoolAllocator(const size_t numChunks, const void* pBase) : BaseAllocator(numChunks * ObjectSize, pBase) { Clear(); }
		inline virtual ~PoolAllocator()
		{
			mpFreeList = nullptr;
		}

		inline virtual void* Allocate(const size_t size) override
		{
			FLX_ASSERT(size > ObjectSize); // Null-Allocation...

			// No Free Slots..
			if (!mpFreeList) OutOfMemory();

			// Get Free Slot
			void* p = mpFreeList;

			// Point to next available Slot:
			mpFreeList = (void**)(*mpFreeList);

			mMemoryUsed += ObjectSize;
			return p;
		}

		inline virtual void Free(void*) override
		{
			*((void**)p) = mpFreeList;

			mpFreeList = (void**)p;

			mMemoryUsed -= ObjectSize;
		}

		inline virtual void Clear() override
		{
			size_t numObjects = (size_t)float(mTotalSize) / ObjectSize;

			union
			{
				void* asVoidPtr;
				char* asCharPtr;
			};

			asVoidPtr = (void*)mpBase;

			mpFreeList = (void**)asVoidPtr;

			void** p = mpFreeList;

			for (size_t i{}; i < numObjects - 1; ++i)
			{
				*p = (void*)((char*)p + ObjectSize);
				p = (void**)*p;
			}

			*p = nullptr;
		}

	private:
		void* mpFreeList;
		// Pointer to an array of void* addresses of free objects
	};

	/* Simple Stack Allocator */
	// -> Memory can be popped & pushed
	class StackAllocator final : public BaseAllocator
	{
	public:
		inline StackAllocator(const size_t size, const void* pBase) : BaseAllocator(size, pBase){}
		inline virtual ~StackAllocator() {};

		inline virtual void* Allocate(const size_t size) override
		{
			FLX_ASSERT(size > 0);

			union
			{
				void* asVoidPtr;
				char* asCharPtr;
			};

			// The top of the stack:
			asVoidPtr = (void*)mpBase;
			asCharPtr += mMemoryUsed;

			if (mMemoryUsed + size > mTotalSize)
			{
				FLX_ASSERT(false);
			}

			mMemoryUsed += size;

			return asVoidPtr;
		}
		inline virtual void Free(void* address) override
		{
			union
			{
				void* asVoidPtr;
				char* asCharPtr;
			};

			asVoidPtr = address;

			size_t amountOfMemory = ((char*)mpBase + mMemoryUsed) - (char*)address;
			mMemoryUsed -= amountOfMemory;
		}
		inline virtual void Clear() override
		{
			mMemoryUsed = 0;
		}
	};
}

#endif