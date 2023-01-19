#pragma once

#ifndef __CORE_POINTER_H_
#define __CORE_POINTER_H_

#include <memory>

namespace Influx
{
	/* We define a Ptr alias, primarily for easy renaming-edits */
	template <typename _T>
	using Ptr = _T*;

	template <typename _T>
	constexpr bool IsNull(const Ptr<_T> p)
	{
		return (p == nullptr);
	}

	template <typename _T>
	using SharedPtr = std::shared_ptr<_T>;

	template <typename _T>
	using UniquePtr = std::unique_ptr<_T>;

	template <typename _T>
	using WeakPtr = std::weak_ptr<_T>;
}

#endif