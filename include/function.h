#pragma once

#ifndef __CORE_FUNCTION_H_
#define __CORE_FUNCTION_H_

#include <functional>
#include <type_traits>

#include "core/container/containers.h"

namespace influx
{
	template <typename _func>
	using function = std::function<_func>;

	// SlimFunction:
#pragma region SlimFunction
	// Based on:
	// https://gist.github.com/twoscomplement/030818a6c38c5a983482dc3a385a3ab8
	template <typename>
	struct slim_function;

	template <typename _r, typename ..._args>
	struct slim_function<_r(_args...)> final
	{
		using Dispatcher = _r(*)(void*, _args...);
		using target_function_ref = _r(_args...);

		Dispatcher m_dispatch;
		void* mp_target;

		_r operator()(_args... args) const
		{
			return m_dispatch(mp_target, args...);
		}

		// Dispatch() is instantiated by the TransientFunction constructor,
		// which will store a pointer to the function in m_Dispatcher.
		template<typename _S>
		static _r Dispatch(void* pTarget, _args&... args)
		{
			return (*(_S*)pTarget)(args...);
		}

		template<typename _t>
		slim_function(_t&& target)
			: m_dispatch(&Dispatch<typename std::decay<_t>::type>)
			, mp_target(&target)
		{
		}

		// Specialize for reference-to-function, to ensure that a valid pointer is 
		// stored.
		slim_function(target_function_ref target)
			: m_dispatch(Dispatch<target_function_ref>)
		{
			static_assert(sizeof(void*) == sizeof target,
				"It will not be possible to pass functions by reference on this platform. "
				"Please use explicit function pointers i.e. foo(target) -> foo(&target)");

			mp_target = (void*)target;
		}
	};

#pragma endregion

	// FunctionList:
#pragma region FunctionList
	template <typename>
	class function_list; // intentionally not defined

	template <typename _ret, typename ..._args>
	class function_list<_ret(_args...)> final
	{
		using function_ptr = function<_ret(_args...)>;
		using function_ptr_list = list<function_ptr>;
		function_ptr_list m_functions;

	public:
		function_list() = default;
		function_list(std::initializer_list<function_ptr> functions)
			: m_functions{functions} 
		{
			
		}

		template <typename ..._params>
		void operator()(_params&... params) const
		{
			for (function_ptr p : m_functions)
			{
				if (p != nullptr)
				{
					p(params...);
				}
			}

			return;
		}

		void operator+=(function_ptr func)
		{
			add(func);
		}

		void add(function_ptr func)
		{
			m_functions.push_back(func);
		}

		void remove(function_ptr func)
		{
			m_functions.remove(func);
		}

		void clear()
		{
			m_functions.clear();
		}

		function_list(const function_list&) = default;
		function_list(function_list&&) = default;
		function_list& operator=(const function_list&) = default;
		function_list& operator=(function_list&&) = default;
		virtual ~function_list() = default;
	};
#pragma endregion
}

#endif