#pragma once

#ifndef _CORE_POINTER_H_
#define _CORE_POINTER_H_

#include <memory.h>

namespace Influx
{
	template <typename _T>
	using SharedPtr = std::shared_ptr<_T>;

	template <typename _T>
	using UniquePtr = std::unique_ptr<_T>;

	template <typename _T>
	using WeakPtr = std::weak_ptr<_T>;
}

#endif