#pragma once

#ifndef _CORE_CAST_H_
#define _CORE_CAST_H_

namespace influx
{
	template <typename _D, typename _type>
	constexpr _D stat_cast(_type v)
	{
		return static_cast<_D>(v);
	}

	template <typename _D, typename _type>
	constexpr _D* stat_cast(_type* p)
	{
		return static_cast<_D*>(p);
	}

	template <typename _D, typename _type>
	constexpr const _D* stat_cast(const _type* p)
	{
		return static_cast<const _D*>(p);
	}

	template <typename _D, typename _type>
	constexpr _D* dyn_cast(_type* p)
	{
		return dynamic_cast<_D*>(p);
	}

	template <typename _D, typename _type>
	constexpr const _D* dyn_cast(_type* p)
	{
		return dynamic_cast<const _D*>(p);
	}
}

#endif