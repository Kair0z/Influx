#pragma once

#ifndef _CORE_CAST_H_
#define _CORE_CAST_H_

namespace Influx
{
	template <typename _Dest, typename _T>
	inline _Dest* StaticCast(_T* p)
	{
		return static_cast<_Dest*>(p);
	}

	template <typename _Dest, typename _T>
	inline const _Dest* StaticCast(const _T* p)
	{
		return static_cast<const _Dest*>(p);
	}

	template <typename _Dest, typename _T>
	inline _Dest* DynamicCast(_T* p)
	{
		return dynamic_cast<_Dest*>(p);
	}

	template <typename _Dest, typename _T>
	inline const _Dest* DynamicCast(_T* p)
	{
		return dynamic_cast<const _Dest*>(p);
	}
}

#endif