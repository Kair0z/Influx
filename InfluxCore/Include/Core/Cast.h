#pragma once

#ifndef _CORE_CAST_H_
#define _CORE_CAST_H_

namespace Influx
{
	template <typename _D, typename _T>
	inline _D StaticCast(_T v)
	{
		return static_cast<_T>(v);
	}

	template <typename _D, typename _T>
	inline _D* StaticCast(_T* p)
	{
		return static_cast<_D*>(p);
	}

	template <typename _D, typename _T>
	inline const _D* StaticCast(const _T* p)
	{
		return static_cast<const _D*>(p);
	}

	template <typename _D, typename _T>
	inline _D* DynamicCast(_T* p)
	{
		return dynamic_cast<_D*>(p);
	}

	template <typename _D, typename _T>
	inline const _D* DynamicCast(_T* p)
	{
		return dynamic_cast<const _D*>(p);
	}
}

#endif