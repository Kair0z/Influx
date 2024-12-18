#pragma once

#ifndef _CORE_LIST_H_
#define _CORE_LIST_H_

#include <list>
#include <vector>

namespace influx
{
	template <typename _T>
	using list = std::list<_T>;

	template <typename _t>
	static std::vector<_t> to_vector(const list<_t>& list)
	{
		std::vector<_t> result{};
		result.reserve(list.size());
		for (const _t& item : list)
		{
			result.push_back(item);
		}
		return result;
	}
}

#endif


