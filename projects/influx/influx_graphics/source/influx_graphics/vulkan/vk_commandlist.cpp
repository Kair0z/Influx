#include "graphics_pch.h"

#include "influx_graphics/vulkan/vk_commandlist.h"
#include "influx_graphics/vulkan/vk_conversion.h"

#include "vk_headers.h"

namespace influx::graphics
{
    vk_commandlist::vk_commandlist(const vk::CommandBuffer& vkbuffer)
        : m_vk_commandbuffer{vkbuffer}
    {
        mp_native = &m_vk_commandbuffer;
    }

    void vk_commandlist::start(command_allocator* allocator, pipeline* init_state)
    {

    }

    void vk_commandlist::clear_rtv(render_target_view* view, const math::vectorf4& clear_value)
    {

    }

    void vk_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
    {

    }

    void vk_commandlist::copy_resource(resource* source, resource* dest)
    {

    }

    void vk_commandlist::end()
    {

    }
}