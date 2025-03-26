#pragma once

#ifndef _CORE_VECTOR_H_
#define _CORE_VECTOR_H_

#include <vector>

namespace influx
{
	template <typename _t>
	using vector = std::vector<_t>;

	namespace vector_helpers
	{
		template <typename _t>
		inline bool contains(const vector<_t>& vec, const _t& value)
		{
			return std::find(vec.cbegin(), vec.cend(), value) != vec.cend();
		}

		template <typename _t, typename _func>
		inline bool contains(const vector<_t>& vec, _func&& func)
		{
			return std::find_if(vec.cbegin(), vec.cend(), func) != vec.cend();
		}
	}
	
	template <typename _t>
	inline static vector<_t>& append(vector<_t>& a, const vector<_t>& b)
	{
		a.reserve(a.size() + b.size());
		for (size_t i = 0u; i < b.size(); ++i)
		{
			a.push_back(b[i]);
		}

		return a;
	}

	template <typename _t>
	inline static vector<_t> merged(const vector<_t>& a, const vector<_t>& b)
	{
		vector<_t> result{};
		const size_t num = a.size() + b.size();
		result.reserve(num);

		for (size_t i = 0u; i < a.size(); ++i)
		{
			result.push_back(a[i]);
		}

		for (size_t i = 0u; i < b.size(); ++i)
		{
			result.push_back(b[i]);
		}
		return result;
	}
}

#endif