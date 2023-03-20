#pragma once

#ifndef _CORE_CAST_H_
#define _CORE_CAST_H_

namespace Influx
{
	template <typename _D, typename _T>
	constexpr _D sCast(_T v)
	{
		return static_cast<_D>(v);
	}

	template <typename _D, typename _T>
	constexpr _D* sCast(_T* p)
	{
		return static_cast<_D*>(p);
	}

	template <typename _D, typename _T>
	constexpr const _D* sCast(const _T* p)
	{
		return static_cast<const _D*>(p);
	}

	template <typename _D, typename _T>
	constexpr _D* dCast(_T* p)
	{
		return dynamic_cast<_D*>(p);
	}

	template <typename _D, typename _T>
	constexpr const _D* dCast(_T* p)
	{
		return dynamic_cast<const _D*>(p);
	}
}

#endif