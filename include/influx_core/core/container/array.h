#pragma once

#ifndef _CORE_ARRAY_H_
#define _CORE_ARRAY_H_

#include <array>

namespace influx
{
	template <typename _T, size_t _N>
	using array = std::array<_T, _N>;
}

#endif