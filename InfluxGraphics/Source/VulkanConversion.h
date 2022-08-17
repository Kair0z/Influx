
#include "VulkanAPI.h"

namespace Influx::Graphics::Conversion
{
	constexpr vk::QueueFlagBits ToVulkan(ERHICommandQueueType type)
	{
		switch (type)
		{
		default:
		case ERHICommandQueueType::Graphics:
			return vk::QueueFlagBits::eGraphics;
		}
	}
}