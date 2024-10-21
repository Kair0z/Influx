#pragma once
#include "core/pointer.h"

namespace influx::rhi
{
    template <typename _t, typename _dxtype, typename _vktype>
    class wrapper
    {
    public:
        using rhi_type = _t;
        using dx12_type = _dxtype;
        using vulkan_type = _vktype;

        virtual ~wrapper();

    private:
        shared_ptr<dx12_type> m_dx12_object;
        shared_ptr<vulkan_type> m_vulkan_object;
    }
}