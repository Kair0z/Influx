#include "VulkanRenderPass.h"
#include "VulkanConversion.h"

namespace Influx::Graphics
{
	VulkanRenderPass::VulkanRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments, const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies)
		: RHIRenderPass(attachments, subpasses, dependencies) {}

	VulkanRenderPass::~VulkanRenderPass()
	{
	}

	vk::SubpassDescription VulkanRenderPass::FromRHI(const RHIRenderSubPassDesc& rhiSubpass)
	{
		vk::SubpassDescription vkResult{};
		vkResult.pipelineBindPoint			= Conversion::ToVulkan(rhiSubpass.PipelineBindPoint);

		assert(false); // How to handle references?
		vkResult.colorAttachmentCount;
		vkResult.pColorAttachments;
		vkResult.pDepthStencilAttachment;
		vkResult.inputAttachmentCount;
		vkResult.pInputAttachments;
		vkResult.preserveAttachmentCount;
		vkResult.pPreserveAttachments;
		vkResult.pResolveAttachments;

		return vkResult;
	}
	vk::AttachmentDescription VulkanRenderPass::FromRHI(const RHIRenderPassAttachmentDesc& rhiAttachment)
	{
		vk::AttachmentDescription vkResult{};
		vkResult.format			= Conversion::ToVulkan(rhiAttachment.Format);
		vkResult.samples		= Conversion::ToVulkan(rhiAttachment.Samples);
		vkResult.loadOp			= Conversion::ToVulkan(rhiAttachment.LoadOperation);
		vkResult.storeOp		= Conversion::ToVulkan(rhiAttachment.StoreOperation);
		vkResult.stencilLoadOp	= Conversion::ToVulkan(rhiAttachment.StencilLoadOperation);
		vkResult.stencilStoreOp = Conversion::ToVulkan(rhiAttachment.StencilStoreOperation);
		vkResult.initialLayout	= Conversion::ToVulkan(rhiAttachment.InitialState);
		vkResult.finalLayout	= Conversion::ToVulkan(rhiAttachment.FinalState);
		return vkResult;
	}
	vk::SubpassDependency VulkanRenderPass::FromRHI(const RHIRenderSubPassDependency& rhiDependency)
	{
		assert(false); // Todo...
		vk::SubpassDependency vkResult{};
		vkResult.srcSubpass;
		vkResult.dstSubpass;
		vkResult.srcStageMask;
		vkResult.dstStageMask;
		vkResult.srcAccessMask;
		vkResult.dstAccessMask;
		vkResult.dependencyFlags;
		return vkResult;
	}

	vk::RenderPass VulkanRenderPass::FromRHI(const vk::Device& device, const std::vector<RHIRenderPassAttachmentDesc>& attachments, const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies)
	{
		// Convert RHI helper desc structs into Vulkan desc structs...
		std::vector<vk::AttachmentDescription> vulkAttachments(attachments.size());
		std::vector<vk::SubpassDescription> vulkSubpasses(subpasses.size());
		std::vector<vk::SubpassDependency> vulkDependencies(dependencies.size());

		for (size_t i = 0; i < vulkAttachments.size(); ++i) vulkAttachments[i] = FromRHI(attachments[i]);
		for (size_t i = 0; i < vulkSubpasses.size(); ++i) vulkSubpasses[i] = FromRHI(subpasses[i]);
		for (size_t i = 0; i < vulkDependencies.size(); ++i) vulkDependencies[i] = FromRHI(dependencies[i]);

		// Create the actual renderpass
		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(vulkAttachments.size());		// Number of attachments used by this render pass
		renderPassInfo.pAttachments = vulkAttachments.data();								// Descriptions of the attachments used by the render pass
		renderPassInfo.subpassCount = static_cast<uint32_t>(vulkSubpasses.size());			
		renderPassInfo.pSubpasses = vulkSubpasses.data();									
		renderPassInfo.dependencyCount = static_cast<uint32_t>(vulkDependencies.size());	// Number of subpass dependencies
		renderPassInfo.pDependencies = vulkDependencies.data();								// Subpass dependencies used by the render pass

		return device.createRenderPass(renderPassInfo, nullptr);
	}
	
	const vk::RenderPass& VulkanRenderPass::GetVulkanRenderPass() const
	{
		return VulkRenderPass;
	}

	void VulkanRenderPass::OnUpdateRHI(GraphicsAPI* api)
	{
		VulkanAPI* vulkanAPI = (VulkanAPI*)api;
		ReconstructVulkanRenderPass(vulkanAPI);
	}

	void VulkanRenderPass::ReconstructVulkanRenderPass(const VulkanAPI* api)
	{
		if (VulkRenderPass) { /*release...*/ }
		
		// Construct a new Renderpass...
		VulkRenderPass = FromRHI(api->GetLogicalDevice(), GetAttachments(), GetSubPasses(), GetDependencies());
	}
}

