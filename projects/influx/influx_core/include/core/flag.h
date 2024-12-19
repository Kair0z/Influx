#pragma once

namespace influx
{
    template <typename _t>
    inline _t set_flag(const _t& base, const _t& flag)
    {
        return (_t)(base | flag);
    }

    template <typename _t>
    inline _t unset_flag(const _t& base, const _t& flag)
    {
        return (_t)(base & ~flag);
    }

    template <typename _t>
    inline _t set_flag(const _t& base, const _t& flag, bool enabled)
    {
        if (enabled)
        {
            return set_flag(base, flag);
        }
        else
        {
            return unset_flag(base, flag);
        }
    }

    template <typename _t>
    inline bool is_flag_set(const _t& base, const _t& flag)
    {
        return base & flag;
    }
}