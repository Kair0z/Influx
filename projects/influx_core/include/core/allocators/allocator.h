#pragma once

#include "core/basetypes.h"

namespace Influx
{
	namespace Internal
	{
		class i_allocator
		{
		public:
			using t_numBytes = size_t;
			using t_address = void*;

			virtual void* allocate(const t_numBytes numBytes) = 0;
			virtual void free(t_address address) = 0;
			virtual void clear() = 0;

			i_allocator(const i_allocator&) = delete;
			i_allocator(i_allocator&&) = delete;
			i_allocator& operator=(const i_allocator&) = delete;
			i_allocator& operator=(i_allocator&&) = delete;
			virtual ~i_allocator() = default;
		};
	}
}
