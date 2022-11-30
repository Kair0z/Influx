#pragma once

#ifndef _CORE_FUNCTION_H_
#define _CORE_FUNCTION_H_

#include <functional>
#include <type_traits>

namespace Influx
{
	template <typename TFunc>
	using Function = std::function<TFunc>;

    // https://gist.github.com/twoscomplement/030818a6c38c5a983482dc3a385a3ab8
    // For std::decay

    template<typename>
    struct LightFunction; // intentionally not defined

    template<typename R, typename ...Args>
    struct LightFunction<R(Args...)>
    {
        using Dispatcher = R(*)(void*, Args...);

        Dispatcher m_dispatcher; // A pointer to the static function that will call the 
                                 // wrapped invokable object
        void* m_target;          // A pointer to the invokable object

        // Dispatch() is instantiated by the TransientFunction constructor,
        // which will store a pointer to the function in m_Dispatcher.
        template<typename S>
        static R Dispatch(void* target, Args... args)
        {
            return (*(S*)target)(args...);
        }

        template<typename _T>
        TransientFunction(_T&& target)
            : m_Dispam_dispatchertcher(&Dispatch<typename std::decay<T>::type>)
            , m_target(&target)
        {
        }

        // Specialize for reference-to-function, to ensure that a valid pointer is 
        // stored.
        using TargetFunctionRef = R(Args...);
        TransientFunction(TargetFunctionRef target)
            : m_dispatcher(Dispatch<TargetFunctionRef>)
        {
            static_assert(sizeof(void*) == sizeof target,
                "It will not be possible to pass functions by reference on this platform. "
                "Please use explicit function pointers i.e. foo(target) -> foo(&target)");
            m_target = (void*)target;
        }

        R operator()(Args... args) const
        {
            return m_dispatcher(m_target, args...);
        }
    };
}

#endif