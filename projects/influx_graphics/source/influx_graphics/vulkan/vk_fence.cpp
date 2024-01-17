#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_fence.h"
#include "vk_headers.h"

namespace influx::graphics
{
    vk_fence::vk_fence(const vk::Fence& vk_fence)
        :m_vk_fence{vk_fence}
    {
        mp_native = &m_vk_fence;
    }

    void vk_fence::queue_signal(uint64 value, command_queue* queue)
    {

    }

    void vk_fence::signal(uint64 value)
    {

    }

    void vk_fence::wait_for_value(uint64 value, wait_handle& handle)
    {

    }
}