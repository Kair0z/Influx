#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_allocator.h"
#include "vk_headers.h"

namespace influx::graphics
{
    vk_command_allocator::vk_command_allocator(const vk::CommandPool& vkpool)
        : m_vk_commandpool{vkpool}
    {
        mp_native = &m_vk_commandpool;
    }
}