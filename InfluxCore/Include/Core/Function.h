#pragma once

#ifndef __CORE_FUNCTION_H_
#define __CORE_FUNCTION_H_

#include <functional>
#include <typeinfo>
#include "Core/Container/Containers.h"

namespace Influx
{
	template <typename _F>
	using Function = std::function<_F>;

	template <typename _F>
	class FunctionList final
	{
		using FunctionPointer = Function<_F>;
		using FunctionPointerList = List<FunctionPointer>;
		FunctionPointerList m_functions;

	public:
		FunctionList() = default;
		FunctionList(std::initializer_list<FunctionPointer> functions)
			: m_functions{functions} 
		{
			
		}

		template <typename ..._Params>
		void operator()(_Params&... params) const
		{
			for (FunctionPointer p : m_functions)
			{
				if (p != nullptr)
				{
					p(params...);
				}
			}

			return;
		}

		void Add(FunctionPointer func)
		{
			m_functions.push_back(func);
		}

		void Remove(FunctionPointer func)
		{
			m_functions.remove(func);
		}

		void Clear()
		{
			m_functions.clear();
		}

		FunctionList(const FunctionList&) = default;
		FunctionList(FunctionList&&) = default;
		FunctionList& operator=(const FunctionList&) = default;
		FunctionList& operator=(FunctionList&&) = default;
		virtual ~FunctionList() = default;
	};
}

#endif