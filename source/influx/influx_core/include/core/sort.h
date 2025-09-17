#pragma once
#include <iterator>
#include "core/result.h"

namespace influx::sort
{
	static constexpr uint32 k_default_max_recursion = 8u;
	using default_comp = std::greater<>;
	using uint64 = unsigned long long;

	struct sort_info final
	{
		sort_info() = default;
		uint32 num_swaps = 0u;
		uint32 num_comparisons = 0u;
	};
	using result = influx::result<sort_info, const char*>;

	// O( sqr(n) )
	template <typename _it, typename _comp = default_comp>
	result bubble_sort(_it begin, _it end, _comp&& comp = {})
	{
		if (begin == end)
			return {};
		
		const bool is_range_inverted = begin > end;
		if (is_range_inverted) 
			return result::make_error("range is inverted! (begin > end) - nothing happened");

		// each iteration, bubble up the biggest number
		// if we didn't swap once, we're done
		sort_info info{};
		uint32 times_swapped = 0u;
		_it curr = begin;
		_it next = curr + 1;
		_it sorted_end = end;
		do
		{
			curr = begin;
			times_swapped = 0u;

			while (curr != sorted_end - 1)
			{
				next = curr + 1;
				info.num_comparisons++;
				if (comp(*curr, *next))
				{
					info.num_swaps++;
					std::swap(*curr, *next);
					times_swapped++;
				}
				++curr;
			}

			sorted_end = curr;
		} while (times_swapped > 0u);

		return info;
	}

	// O( n			) - best case
	// O( sqr(n)	) - worst case
	template <typename _it, typename _comp = default_comp>
	result insert_sort(_it begin, _it end, _comp&& comp = {})
	{
		if (begin == end)
			return {};

		if (end - begin <= 1)
			return result::make_error("range is not bigger than 1! - nothing happened");

		const bool is_range_inverted = begin > end;
		if (is_range_inverted)
			return result::make_error("range is inverted! (begin > end) - nothing happened");

		sort_info info{};
		for (_it i = begin + 1; i != end; ++i)
		{
			_it rev = i; // reverse pointer
			do
			{
				rev--;
				info.num_comparisons++;
				while (comp(*rev, *(rev + 1)))
				{
					info.num_swaps++;
					std::swap(*rev, *(rev + 1));
					rev -= (int)(rev > begin);
				}

			} while (rev > begin);
		}
		return info;
	}

	// O( sqr(n) )
	template <typename _it, typename _comp = default_comp>
	result select_sort(_it begin, _it end, _comp&& comp = {})
	{
		if (begin == end)
			return {};

		const bool is_range_inverted = begin > end;
		if (is_range_inverted)
			return result::make_error("range is inverted! (begin > end) - nothing happened");

		sort_info info{};

		// 1. find the minimum in the unsorted range
		// 2. then swap the minimum in the unsorted range to unsorted start
		_it unsorted_start	= begin;
		_it min	= unsorted_start;

		while (unsorted_start < end)
		{
			min = unsorted_start;
			for (_it i = unsorted_start; i < end; ++i)
			{
				info.num_comparisons++;
				min = ( comp(*i, *min) ? min : i);
			}

			info.num_swaps++;
			std::swap(*unsorted_start, *min);
			unsorted_start++;
		}
		return info;
	}

	// O( log(n) )
	template <typename _it, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result merge_sort(_it begin, _it end, _comp&& comp = {})
	{
		static constexpr uint32 k_max_recursion = _k;

		sort_info info{};
		// todo
		return result::make_error("todo: noimpl!");
	}

	// O( sqr(n)		) - worst case
	// O( n * log(n)	) - avg case
	template <typename _it, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result quick_sort(_it begin, _it end, _comp&& comp = {})
	{
		static constexpr uint32 k_max_recursion = _k;
		sort_info info{};
		// todo
		return result::make_error("todo: noimpl!");
	}

	// c array overloads
	template <typename _t, uint64 _n, typename _comp = default_comp>
	result bubble_sort(_t(&arr)[_n], _comp&& comp = _comp{}) { return bubble_sort(std::begin(arr), std::end(arr), comp); }
	template <typename _t, uint64 _n, typename _comp = default_comp>
	result insert_sort(_t(&arr)[_n], _comp&& comp = _comp{}) { return insert_sort(std::begin(arr), std::end(arr), comp); }
	template <typename _t, uint64 _n, typename _comp = default_comp>
	result select_sort(_t(&arr)[_n], _comp&& comp = _comp{}) { return select_sort(std::begin(arr), std::end(arr), comp); }

	template <typename _t, uint64 _n, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result merge_sort(_t(&arr)[_n], _comp&& comp = _comp{}) { return merge_sort<std::iterator, _comp, _k>(std::begin(arr), std::end(arr), comp); }
	template <typename _t, uint64 _n, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result quick_sort(_t(&arr)[_n], _comp&& comp = _comp{}) { return quick_sort<std::iterator, _comp, _k>(std::begin(arr), std::end(arr), comp); }

	// STL container overloads
	template <typename _t, typename _comp = default_comp>
	result bubble_sort(_t& container, _comp&& comp = _comp{}) { return bubble_sort(std::begin(container), std::end(container), comp); }
	template <typename _t, typename _comp = default_comp>
	result insert_sort(_t& container, _comp&& comp = _comp{}) { return insert_sort(std::begin(container), std::end(container), comp); }
	template <typename _t, typename _comp = default_comp>
	result select_sort(_t& container, _comp&& comp = _comp{}) { return select_sort(std::begin(container), std::end(container), comp); }

	template <typename _t, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result merge_sort(_t& container, _comp&& comp = _comp{}) { return merge_sort(std::begin(container), std::end(container), comp); }
	template <typename _t, typename _comp = default_comp, uint32 _k = k_default_max_recursion>
	result quick_sort(_t& container, _comp&& comp = _comp{}) { return quick_sort(std::begin(container), std::end(container), comp); }
}