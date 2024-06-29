#pragma once

#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_

#include <vector>

namespace influx
{
	template <typename _t>
	using vector = std::vector<_t>;
}

#endif