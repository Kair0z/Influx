#pragma once

#ifndef __GR_TYPES_H_
#define __GR_TYPES_H_

#define __USE_INFLUX_CORE 1

#include "RHITypes.h"

#if __USE_INFLUX_CORE
#include "Core/BasicTypes.h"
#include "Core/String.h"
#include "Core/Container/Vector.h"
#include "Core/Container/List.h"
#include "Core/Container/Queue.h"
#include "Core/Singleton/Singleton.h"
#include "Core/Function.h"
#include "Core/Math/Vector.h"
#include "Core/Cast.h"
#else
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <functional>
#endif

namespace influx
{
#if !__USE_INFLUX_CORE
	using uint = unsigned int;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using String	= std::string;
	using WString	= std::wstring;

	template <typename _t> using Vector		= std::vector<_t>;
	template <typename _t> using List		= std::list<_t>;
	template <typename _t> using Queue		= std::queue<_t>;
	template <typename _t> using Function	= std::function<_t>;

	template <class _t>
	class Singleton
	{
	public:
		inline static _t& Get()
		{
			return s_instance;
		}

	private:
		static _t s_instance;
	};

	template <class _t>
	_t Singleton<_t>::s_instance = _t{};
#endif
}

#endif

