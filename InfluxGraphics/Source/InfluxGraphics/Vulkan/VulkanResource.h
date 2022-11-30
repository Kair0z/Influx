#pragma once

#include "VulkanAPI.h"
#include "../RHIResource.h"

namespace Influx::Graphics
{
	class VulkanResource final : public RHIResource
	{
	public:

	};

	class VulkanTexture final : public RHITexture
	{
	public:
		VulkanTexture(RHITextureDescription textureDesc);
		virtual ~VulkanTexture() = default;

		vk::Image GetVulkanImage() const;

	private:
		vk::Image VulkImage;
	};
}


