#pragma once

#include <unordered_map>
#include <map>

namespace Influx
{
	template <typename K, typename V>
	using Unordered_Map = std::unordered_map<K, V>;

	template <typename K, typename V>
	using Map = std::map<K, V>;
}