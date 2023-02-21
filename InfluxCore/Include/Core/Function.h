#pragma once

#ifndef __CORE_FUNCTION_H_
#define __CORE_FUNCTION_H_

#include <functional>
#include <type_traits>
#include "Core/Container/Containers.h"

namespace Influx
{
	template <typename _F>
	using Function = std::function<_F>;

	// SlimFunction:
#pragma region SlimFunction
	// Based on:
	// https://gist.github.com/twoscomplement/030818a6c38c5a983482dc3a385a3ab8
	template <typename>
	struct SlimFunction;

	template <typename _R, typename ..._Args>
	struct SlimFunction<_R(_Args...)> final
	{
		using Dispatch = _R(*)(void*, _Args...);
		using TargetFunctionRef = _R(_Args...);

		Dispatch m_dispatch;
		void* mp_target;

		_R operator()(_Args... args) const
		{
			return m_dispatch(mp_target, args...);
		}

		// Dispatch() is instantiated by the TransientFunction constructor,
		// which will store a pointer to the function in m_Dispatcher.
		template<typename _S>
		static _R Dispatch(void* pTarget, _Args&... args)
		{
			return (*(_S*)pTarget)(args...);
		}

		template<typename _T>
		SlimFunction(_T&& target)
			: m_dispatch(&Dispatch<typename std::decay<_T>::type>)
			, mp_target(&target)
		{
		}

		// Specialize for reference-to-function, to ensure that a valid pointer is 
		// stored.
		SlimFunction(TargetFunctionRef target)
			: m_dispatch(Dispatch<TargetFunctionRef>)
		{
			static_assert(sizeof(void*) == sizeof target,
				"It will not be possible to pass functions by reference on this platform. "
				"Please use explicit function pointers i.e. foo(target) -> foo(&target)");
			m_Target = (void*)target;
		}
	};

#pragma endregion

	template <typename>
	class FunctionList; // intentionally not defined

	template <typename _R, typename ..._Args>
	class FunctionList<_R(_Args...)> final
	{
		using FunctionPointer = Function<_R(_Args...)>;
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