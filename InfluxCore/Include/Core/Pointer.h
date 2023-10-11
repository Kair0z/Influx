#pragma once

#ifndef __CORE_POINTER_H_
#define __CORE_POINTER_H_

#include <memory>

namespace influx
{
	template <typename _t>
	using ptr = _t*;

	template <typename _t>
	constexpr bool is_null(const ptr<_t> p)
	{
		return (p == nullptr);
	}

	template <typename _t>
	using shared_ptr = std::shared_ptr<_t>;

	template <typename _t>
	using uni_ptr = std::unique_ptr<_t>;

	template <typename _t>
	using weak_ptr = std::weak_ptr<_t>;
}

#endif