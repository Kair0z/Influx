#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_resource.h"
#include "vk_headers.h"

namespace influx::graphics
{
    vk_resource::vk_resource(const vk::Image& image)
        : resource()
        , m_vk_image{image}
    {
        mp_native = &m_vk_image;
    }
}