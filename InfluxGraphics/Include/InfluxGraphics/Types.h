#pragma once

#ifndef __GR_TYPES_H_
#define __GR_TYPES_H_

#define USE_INFLUX_CORE 1

#if USE_INFLUX_CORE
#include "Core/BasicTypes.h"
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

namespace Influx
{
#if !USE_INFLUX_CORE
	using uint = unsigned int;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

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

