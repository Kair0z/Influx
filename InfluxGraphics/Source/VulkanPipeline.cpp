#include "VulkanPipeline.h"
#include "VulkanConversion.h"

namespace Influx::Graphics
{
	RHIGraphicsPipelineLayout* VulkanAPI::CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const
	{
		VulkanGraphicsPipelineLayout* newVulkanGfxPipelineLayout = new VulkanGraphicsPipelineLayout();

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType					= vk::StructureType::ePipelineLayoutCreateInfo;;
		pipelineLayoutInfo.setLayoutCount			= 0; // Optional
		pipelineLayoutInfo.pSetLayouts				= nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount	= 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges		= nullptr; // Optional

		newVulkanGfxPipelineLayout->VulkPipelineLayout = GetLogicalDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);

		return newVulkanGfxPipelineLayout;
	}

	RHIGraphicsPipeline* VulkanAPI::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const
	{
		VulkanGraphicsPipeline* newVulkanGfxPipeline = new VulkanGraphicsPipeline();
		newVulkanGfxPipeline->PipelinelayoutReference = pipelineLayoutReference;
		newVulkanGfxPipeline->ConstructionDescription = constructionArgs;
		
		// Vertex Input
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
		vertexInputInfo.vertexBindingDescriptionCount;
		vertexInputInfo.pVertexBindingDescriptions;
		vertexInputInfo.vertexAttributeDescriptionCount;
		vertexInputInfo.pVertexAttributeDescriptions;
		// ---
		
		// Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType						= vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
		inputAssemblyInfo.topology					= Conversion::ToVulkan(ERHIPrimitiveTopology::TriangleList);
		inputAssemblyInfo.primitiveRestartEnable	= VK_FALSE;
		// ---
		
		// Viewport & Scissors
		// .. There's an option to have it statically baked into the pipeline, but we're setting it up as a Dynamic State -> changeable in commandbuffer
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
		std::vector<vk::DynamicState> dynamicStates		
			= { vk::DynamicState::eViewport, vk::DynamicState::eScissor /* Todo, add more dynamic states... */};
		
		dynamicStateInfo.sType							= vk::StructureType::ePipelineDynamicStateCreateInfo;
		dynamicStateInfo.dynamicStateCount				= (uint32_t)dynamicStates.size();
		dynamicStateInfo.pDynamicStates					= dynamicStates.data();
		
		vk::PipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
		viewportInfo.viewportCount = 1; // This is still statically baked into the pipeline though...
		viewportInfo.scissorCount = 1;
		// ---
		
		// Rasterizer:
		vk::PipelineRasterizationStateCreateInfo rasterInfo{};
		rasterInfo.sType					= vk::StructureType::ePipelineRasterizationStateCreateInfo;
		rasterInfo.depthClampEnable			= constructionArgs.bRasterDepthClipEnable;
		rasterInfo.rasterizerDiscardEnable; // ! Requires GPU Feature !
		rasterInfo.polygonMode				= Conversion::ToVulkan(constructionArgs.RasterFillMode);
		rasterInfo.lineWidth;				// ! Requires wideLines GPU Feature !
		rasterInfo.cullMode					= Conversion::ToVulkan(constructionArgs.RasterCullMode);
		rasterInfo.frontFace				= vk::FrontFace::eClockwise;
		rasterInfo.depthBiasEnable			= constructionArgs.RasterDepthBias > 0;
		rasterInfo.depthBiasConstantFactor	= (float)constructionArgs.RasterDepthBias;
		rasterInfo.depthBiasClamp			= (float)constructionArgs.RasterMaxDepthBias;
		rasterInfo.depthBiasSlopeFactor;

		// Multisampling:
		vk::PipelineMultisampleStateCreateInfo multiSampleInfo{};
		multiSampleInfo.sType					= vk::StructureType::ePipelineMultisampleStateCreateInfo;
		multiSampleInfo.sampleShadingEnable		= false;
		multiSampleInfo.rasterizationSamples	= Conversion::ToVulkan(ERHISampleCount::_1);
		multiSampleInfo.minSampleShading		= 1.0f;		// optional
		multiSampleInfo.pSampleMask				= nullptr; // optional
		multiSampleInfo.alphaToCoverageEnable	= false;   // optional
		multiSampleInfo.alphaToOneEnable		= false;	// optional

		// Depth / Stencil
		vk::PipelineDepthStencilStateCreateInfo dsInfo{};
		dsInfo.sType		= vk::StructureType::ePipelineDepthStencilStateCreateInfo;
		// Todo...

		// Blend state
		vk::PipelineColorBlendStateCreateInfo  blendInfo{};
		blendInfo.sType				= vk::StructureType::ePipelineColorBlendStateCreateInfo;
		blendInfo.logicOpEnable		= false;
		blendInfo.logicOp			= vk::LogicOp::eCopy;
		blendInfo.attachmentCount	= 1;		// This could be a problem... todo
		blendInfo.pAttachments		= nullptr;	// This could be a problem... todo
		blendInfo.blendConstants[0] = 0.0f; // Optional
		blendInfo.blendConstants[1] = 0.0f; // Optional
		blendInfo.blendConstants[2] = 0.0f; // Optional
		blendInfo.blendConstants[3] = 0.0f; // Optional

		// Shaders:
		//pipelineInfo.stageCount = 2;
		//RHIShader** allShaders = new RHIShader*[pipelineInfo.stageCount] { constructionArgs.PixelShader, constructionArgs.VertexShader };
		//for (int i = 0; i < 2; ++i)
		//{
		//	RHIShader* shader = allShaders[i];
		//	pipelineInfo.pStages; // Todo...
		//}

		// Create the Pipeline:
		vk::GraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
		pipelineInfo.renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.layout = ((VulkanGraphicsPipelineLayout*)pipelineLayoutReference)->VulkPipelineLayout;

		pipelineInfo.pVertexInputState		= &vertexInputInfo;
		pipelineInfo.pInputAssemblyState	= &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState	= &rasterInfo;
		pipelineInfo.pMultisampleState		= &multiSampleInfo;
		pipelineInfo.pDepthStencilState		= &dsInfo;
		pipelineInfo.pColorBlendState		= &blendInfo;
		pipelineInfo.pDynamicState			= &dynamicStateInfo;

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

