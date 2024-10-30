#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_resource.h"
#include "vk_headers.h"

namespace influx::graphics
{
    vk_resource::vk_resource(const vk::Image& image)
        : resource()
        , m_vk_image{ image }
    {
        mp_native = &m_vk_image;
    }

    void* vk_resource::map(const map_args& args)
    {
        return nullptr;
    }

    void vk_resource::unmap(const map_args& args)
    {
    }

    void vk_resource::on_set_name(const debug_name& name)
    {
        
    }
}