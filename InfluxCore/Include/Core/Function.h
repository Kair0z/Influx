#pragma once

#ifndef __CORE_FUNCTION_H_
#define __CORE_FUNCTION_H_

#include <functional>
#include "Core/Container/Containers.h"

namespace Influx
{
	template <typename _F>
	using Function = std::function<_F>;

	template <typename _F>
	class FunctionList final
	{
		using FunctionPointer = Function<_F>;

	public:
		void operator()() const
		{
			for (FunctionPointer p : m_functions)
			{
				p();
			}
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

	private:
		List<FunctionPointer> m_functions;
	};
}

#endif