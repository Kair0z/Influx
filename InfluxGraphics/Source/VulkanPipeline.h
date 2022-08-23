#pragma once

#include "VulkanAPI.h"
#include "RHIPipeline.h"

namespace Influx::Graphics
{
	class VulkanGraphicsPipelineLayout final : public RHIGraphicsPipelineLayout
	{
		friend class VulkanAPI;

	public:
		virtual ~VulkanGraphicsPipelineLayout();

	private:
		vk::PipelineLayout VulkPipelineLayout;
	};

	class VulkanGraphicsPipeline final : public RHIGraphicsPipeline
	{
		friend class VulkanAPI;

	public:
		virtual ~VulkanGraphicsPipeline();

	private:
		vk::Pipeline VulkGfxPipeline;
	};
}



