#include "VulkanResource.h"

namespace Influx::Graphics
{
    vk::Image VulkanTexture::GetVulkanImage() const
    {
        return VulkImage;
    }
}