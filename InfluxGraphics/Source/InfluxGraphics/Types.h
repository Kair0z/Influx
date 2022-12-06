#pragma once

#ifndef _INFLUX_GRAPHICS_TYPES_H_
#define _INFLUX_GRAPHICS_TYPES_H_

#define USE_INFLUX_CORE 1

#if USE_INFLUX_CORE
#include "Core/String.h"
#include "Core/Container/Vector.h"
#include "Core/Container/List.h"
#include "Core/Container/Queue.h"
#include "Core/Singleton/Singleton.h"
#include "Core/Function.h"

#include "Core/Math/Vector.h"
#else
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <functional>
#endif

namespace
{
	using uint = unsigned int;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

#if USE_INFLUX_CORE
	using String	= Influx::String;
	using WString	= Influx::WString;

	template <typename _T> using Vector		= Influx::Vector<_T>;
	template <typename _T> using List		= Influx::List<_T>;
	template <typename _T> using Queue		= Influx::Queue<_T>;
	template <typename _T> using Singleton	= Influx::Singleton<_T>;
	template <typename _T> using Function	= Influx::Function<_T>;
#else
	using String	= std::string;
	using WString	= std::wstring;

	template <typename _T> using Vector		= std::vector<_T>;
	template <typename _T> using List		= std::list<_T>;
	template <typename _T> using Queue		= std::queue<_T>;
	template <typename _T> using Function	= std::function<_T>;

	template <class _T>
	class Singleton
	{
	public:
		inline static _T& Get()
		{
			return s_instance;
		}

	private:
		static _T s_instance;
	};

	template <class _T>
	_T Singleton<_T>::s_instance = _T{};
#endif
}

#endif

