#pragma once

#include "Core/BasicTypes.h"

namespace Influx
{
	namespace Internal
	{
		class IAllocator
		{
		public:
			using t_numBytes = uint64;
			using t_address = void*;

			virtual void* Allocate(const t_numBytes numBytes) = 0;
			virtual void Free(t_address address) = 0;
			virtual void Clear() = 0;

			IAllocator(const IAllocator&) = delete;
			IAllocator(IAllocator&&) = delete;
			IAllocator& operator=(const IAllocator&) = delete;
			IAllocator& operator=(IAllocator&&) = delete;
			virtual ~IAllocator() = default;
		};
	}

	template <typename _T>
	class Allocator final : Internal::IAllocator
	{
	public:
		Allocator() = default;
		template <class _U> Allocator(const Allocator<_U>&) = default;

		virtual _T* Allocate(const t_numBytes numBytes) override final
		{
			//Platform::Allocate(numBytes);
		}

		virtual void Free(_T* address) override final
		{
			//Platform::Free(address);
		}

		virtual void Clear() override final
		{

		}
	};
}
