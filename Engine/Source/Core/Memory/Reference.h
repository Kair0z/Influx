#pragma once
#include <memory>

namespace Influx
{
	// [CRINGE]: Implement this ourselves...
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T>
	using Own = std::unique_ptr<T>;

	template <typename T>
	using WeakRef = std::weak_ptr<T>;

	template <typename T>
	using Ptr = T*;

	template <typename T>
	using SPtr = Ref<T>;

	template <typename T>
	using UPtr = Own<T>;

	template <typename T>
	using WPtr = WeakRef<T>;

	template <typename T>
	inline Ref<T> MakeRef(Ptr<T> p)
	{
		return std::make_shared<T>(p);
	}
}
