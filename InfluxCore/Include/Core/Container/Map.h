#pragma once

#ifndef _CORE_MAP_H_
#define _CORE_MAP_H_

#include <unordered_map>
#include <map>

namespace Influx
{
	template <typename _K1, typename _K2>
	using Pair = std::pair<_K1, _K2>;

	template <typename _K1, typename _K2>
	struct PairHash final
	{
		std::size_t operator()(const Pair<_K1, _K2>& pair) const
		{
			return std::hash<_K1>()(pair.first) ^ std::hash<_K2>()(pair.second);
		}
	};

	template <typename _K, typename _T, class _H = std::hash<_K>, class _Eq = std::equal_to<_K>>
	using UMap = std::unordered_map<_K, _T, _H, _Eq>;

	template <typename K, typename V>
	using Map = std::map<K, V>;
}

#endif