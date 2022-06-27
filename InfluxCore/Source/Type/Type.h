#pragma once

namespace Influx
{
	/* For now, just a static cast.. */
	// [NOT-BASED]
	template <typename ToType, typename T>
	inline ToType* Cast(T* p)
	{
		return static_cast<ToType*>(p);
	}

	template <typename ToType, typename T>
	inline const ToType* Cast(const T* p)
	{
		return static_cast<const ToType*>(p);
	}
}