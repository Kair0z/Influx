#pragma once

#ifndef _CORE_MAP_H_
#define _CORE_MAP_H_

#include <unordered_map>
#include <map>

namespace influx
{
	template <typename _key1, typename _key2>
	using pair = std::pair<_key1, _key2>;

	template <typename _key1, typename _key2>
	struct pair_hash final
	{
		std::size_t operator()(const pair<_key1, _key2>& pair) const
		{
			return std::hash<_key1>()(pair.first) ^ std::hash<_key2>()(pair.second);
		}
	};

	template <typename _K, typename _t, class _H = std::hash<_K>, class _Eq = std::equal_to<_K>>
	using umap = std::unordered_map<_K, _t, _H, _Eq>;

	template <typename K, typename V>
	using map = std::map<K, V>;
}

#endif