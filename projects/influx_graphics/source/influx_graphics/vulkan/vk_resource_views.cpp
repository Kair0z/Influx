#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_resource_views.h"
#include "vk_headers.h"

namespace influx::graphics
{
    vk_render_target_view::vk_render_target_view(const vk::ImageView& vk_view)
        : render_target_view(nullptr)
        , m_vk_view{vk_view}
    {
    }
}