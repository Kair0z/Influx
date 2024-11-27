#pragma once

#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_

#include <vector>

namespace influx
{
	template <typename _t>
	using vector = std::vector<_t>;

	template <typename _t>
	inline static void append(vector<_t>& a, const vector<_t>& b)
	{
		a.reserve(a.size() + b.size());
		for (size_t i = 0u; i < b.size(); ++i)
		{
			a.push_back(b[i]);
		}
	}
}

#endif