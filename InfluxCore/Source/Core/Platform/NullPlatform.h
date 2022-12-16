#pragma once

#ifndef _CORE_PLATFORM_NULL_H_
#define _CORE_PLATFORM_NULL_H_

namespace Influx::Platform
{
	template <typename _T>
	inline _T* Allocate(const size_t numBytes)
	{
		static_assert(false, "Null Platform!");
		return nullptr;
	}

	template <typename _T>
	inline void Free(_T* address)
	{
		static_assert(false, "Null Platform!");
	}
}

#endif