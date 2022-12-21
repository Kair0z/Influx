#pragma once

#ifndef _CORE_ARRAY_H_
#define _CORE_ARRAY_H_

#include <array>

namespace Influx
{
	template <typename _T, size_t _N>
	using Array = std::array<_T, _N>;
}

#endif