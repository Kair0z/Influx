#include "VulkanPipeline.h"

namespace Influx::Graphics
{
	RHIGraphicsPipelineLayout* VulkanAPI::CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const
	{
		VulkanGraphicsPipelineLayout* newVulkanGfxPipelineLayout = new VulkanGraphicsPipelineLayout();

		return newVulkanGfxPipelineLayout;
	}

	RHIGraphicsPipeline* VulkanAPI::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const
	{
		VulkanGraphicsPipeline* newVulkanGfxPipeline = new VulkanGraphicsPipeline();
		newVulkanGfxPipeline->PipelinelayoutReference = pipelineLayoutReference;
		newVulkanGfxPipeline->ConstructionDescription = constructionArgs;

		vk::GraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
		
		// Root Signature
		pipelineInfo.layout = ((VulkanGraphicsPipelineLayout*)pipelineLayoutReference)->VulkPipelineLayout;

		// InputLayout
		pipelineInfo.pVertexInputState;

		// Primitive Topology Type
		pipelineInfo.pInputAssemblyState->topology;
		
		// Shaders:
		pipelineInfo.stageCount = 2;
		RHIShader** allShaders = new RHIShader*[pipelineInfo.stageCount] { constructionArgs.PixelShader, constructionArgs.VertexShader };
		for (int i = 0; i < 2; ++i)
		{
			RHIShader* shader = allShaders[i];
			pipelineInfo.pStages; // Todo...
		}
		
		// Rasterizer:
		pipelineInfo.pRasterizationState;
		pipelineInfo.pMultisampleState;

		pipelineInfo.pViewportState;
		pipelineInfo.pDepthStencilState;
		pipelineInfo.pColorBlendState;
		pipelineInfo.pDynamicState;

		// Renderpass:
		pipelineInfo.renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	// Optional
		pipelineInfo.basePipelineIndex = -1;				// Optional

		vk::PipelineCache pipelineCache = VK_NULL_HANDLE;
		newVulkanGfxPipeline->VulkGfxPipeline = MainDevice.VkLogicalDevice.createGraphicsPipeline(pipelineCache, pipelineInfo, nullptr).value;

		return newVulkanGfxPipeline;
	}

	VulkanGraphicsPipelineLayout::~VulkanGraphicsPipelineLayout()
	{
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
	}
}

