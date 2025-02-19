#pragma once

#ifndef _CORE_ARRAY_H_
#define _CORE_ARRAY_H_

#include <array>

namespace influx
{
	template <typename _T, size_t _N>
	using array = std::array<_T, _N>;

	template <typename _t, size_t _n>
	using stat_array = std::array<_t, _n>;
}

#endif