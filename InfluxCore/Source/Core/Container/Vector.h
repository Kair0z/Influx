#pragma once

#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_

#include <vector>

namespace Influx
{
	template <typename _T>
	using Vector = std::vector<_T>;
}

#endif