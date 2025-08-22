#pragma once

#ifndef _CORE_LIST_H_
#define _CORE_LIST_H_

#include <list>
#include <vector>
#include <algorithm>

#include "core/result.h"

namespace influx
{
	template <typename _t>
	class list : public std::list<_t>
	{
	public:
		influx::result<_t const*, const char*> find(const _t& value) const
		{
			using result_type = influx::result<_t const*, const char*>;
			auto found = std::find(this->cbegin(), this->cend(), value);
			if (found != this->cend())
			{
				return &(*found);
			}
			else return result_type::make_error("value not found in list!");
		}
	};

	template <typename _t>
	inline static std::vector<_t> to_vector(const list<_t>& list)
	{
		std::vector<_t> result{};
		result.reserve(list.size());
		for (const _t& item : list)
		{
			result.push_back(item);
		}
		return result;
	}

	template <typename _t>
	inline static bool remove(list<_t>& list, const _t& value)
	{
		auto found = std::find(list.cbegin(), list.cend(), value);
		if (found != list.cend())
		{
			list.erase(found);
			return true;
		}

		return false;
	}
}

#endif


