#pragma once

#ifndef _CORE_MAP_H_
#define _CORE_MAP_H_

#include <unordered_map>
#include <map>

namespace Influx
{
	template <typename K, typename V>
	using UMap = std::unordered_map<K, V>;

	template <typename K, typename V>
	using Map = std::map<K, V>;
}

#endif