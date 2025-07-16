
#ifndef _CORE_CONTAINERS_H_
#define _CORE_CONTAINERS_H_

#include "core/result.h"

#define USE_STL 1
#if USE_STL
#include <array>
#include <string>
#endif

namespace influx
{
    using size = size_t;

    // [declarations]
    namespace ctr
    {
        template <typename _ct, typename _t>
        static bool contains(const _ct& container, const _t& value);

        // template <typename _ct, typename _pred>
        // static bool contains(const _ct& container, _pred&& predicate);

        /* 
            pushes a piece of data 
            the container type decides whatever way is more convenient
            (e.g. push_back()) 
        */
        template <typename _ct, typename _t>
        static result<> push(_ct& container, const _t& value);

        template <typename _ct, typename _t>
        static result<> remove(_ct& container, const _t& value);

        /* merges data from the source into the dest container */
        template <typename _ct>
        static result<> merge(_ct& dest, const _ct& source);

        template <typename _ct, typename _t>
        static result<> fill(_ct& container, const _t& value);

        template <typename _ct>
        static result<> swap(_ct& a, _ct& b);

        template <typename _ct>
        static result<> get_num(const _ct& container);

        template <typename _ct>
        static result<> get_capacity(const _ct& container);

        template <typename _ct, typename _t>
        static result<int32> get_index(const _ct& container, const _t& value);

        template <typename _t>
        class base
        {
        public:

        };
    }
    
    // [static array] (std::array)
    template <typename _t, size _n>
    class static_array
        : public std::array<_t, _n>
        , public ctr::base<_t>
    {
    public:
        static_array() = default;
    };

    // [dynamic array] (std::vector)
    template <typename _t>
    class dynamic_array 
        : public std::vector<_t>
        , public ctr::base<_t>
    {
    public:
        dynamic_array() = default;
    };

    // [aliases]
    template <typename _T, size _N>
    using array = static_array<_T, _N>;
    template <typename _t>
    using vec = dynamic_array<_t>;

    // [impl]
    namespace ctr
    {
        // [contains]
        template <typename _t, size _n>
        static bool contains(const static_array<_t, _n>& container, const _t& value)
        {
            for (const _t& val : container)
            {
                if (val == value) return true;
            }
            return false;
        }

        template <typename _t>
        static bool contains(const dynamic_array<_t>& container, const _t& value)
        {
            for (const _t& val : container)
            {
                if (val == value) return true;
            }
            return false;
        }

        // [push]
        template <typename _t, size _n>
        static result<> push(static_array<_t, _n>& container, const _t& value)
        {
            static_assert(false, "static arrays cannot allocate more than their static size!");
        }
        
        template <typename _t>
        static result<> push(dynamic_array<_t>& container, const _t& value)
        {
            using result_type = result<>;
            container.push_back(value);
            return {};
        }

        // [merge]
        template <typename _t, size _n>
        static result<> merge(static_array<_t, _n>& dest, const static_array<_t, _n>& source)
        {
            using result_type = result<>;
            return {};
        }

        template <typename _t>
        static result<> merge(dynamic_array<_t>& dest, const dynamic_array<_t>& source)
        {
            using result_type = result<>;
            return {};
        }
    }
}

#endif