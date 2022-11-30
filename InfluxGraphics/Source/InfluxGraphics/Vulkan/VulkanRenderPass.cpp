#include "VulkanRenderPass.h"
#include "VulkanConversion.h"

namespace Influx::Graphics
{
	VulkanRenderPass::VulkanRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments, const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies)
		: RHIRenderPass(attachments, subpasses, dependencies) {}

	VulkanRenderPass::~VulkanRenderPass()
	{
	}

	vk::AttachmentReference VulkanRenderPass::FromRHI(const RHIRenderSubPassDesc::AttachmentRef& rhiAttachmentRef)
	{
		vk::ImageLayout layoutFromRef{};
		switch (rhiAttachmentRef.Layout)
		{
		default: assert(false); break; // Not yet implemented...
		case RHIRenderSubPassDesc::AttachmentRef::ELayout::ColorAttachmentOptimal: layoutFromRef = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference vkResult{};
		vkResult.attachment = rhiAttachmentRef.AttachmentIndex;
		vkResult.layout = layoutFromRef;
		return vkResult;
	}

	vk::SubpassDescription VulkanRenderPass::FromRHI(const RHIRenderSubPassDesc& rhiSubpass
		, std::vector<vk::AttachmentReference>& colorAttachments, std::vector<vk::AttachmentReference>& depthStencilAttachments, std::vector<vk::AttachmentReference>& inputAttachments, std::vector<vk::AttachmentReference>& resolveAttachments)
	{
		vk::SubpassDescription vkResult{};
		vkResult.pipelineBindPoint			= Conversion::ToVulkan(rhiSubpass.PipelineBindPoint);

		for (size_t i = 0; i < rhiSubpass.AttachmentReferences.size(); ++i)
		{
			const RHIRenderSubPassDesc::AttachmentRef& reference = rhiSubpass.AttachmentReferences[i];

			switch (reference.Type)
			{
			case RHIRenderSubPassDesc::AttachmentRef::EType::Color:			colorAttachments.push_back(FromRHI(reference)); break;
			case RHIRenderSubPassDesc::AttachmentRef::EType::DepthStencil:	depthStencilAttachments.push_back(FromRHI(reference)); break;
			case RHIRenderSubPassDesc::AttachmentRef::EType::Input:			inputAttachments.push_back(FromRHI(reference)); break;
			case RHIRenderSubPassDesc::AttachmentRef::EType::Resolve:		resolveAttachments.push_back(FromRHI(reference)); break;
			default: assert(false); break;
			}
		}

		vkResult.colorAttachmentCount		= (uint32_t)colorAttachments.size();
		vkResult.pColorAttachments			= colorAttachments.data();
		vkResult.pDepthStencilAttachment	= (depthStencilAttachments.size() > 0) ? depthStencilAttachments.data() : nullptr;
		vkResult.inputAttachmentCount		= (uint32_t)inputAttachments.size();
		vkResult.pInputAttachments			= inputAttachments.data();
		vkResult.preserveAttachmentCount;
		vkResult.pPreserveAttachments;
		vkResult.pResolveAttachments		= resolveAttachments.data();

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

	vk::RenderPass VulkanRenderPass::FromRHI(const vk::Device& device, 
		const std::vector<RHIRenderPassAttachmentDesc>& attachments, 
		const std::vector<RHIRenderSubPassDesc>& subpasses, 
		const std::vector<RHIRenderSubPassDependency>& dependencies)
	{
		// Convert RHI helper desc structs into Vulkan desc structs...
		std::vector<vk::AttachmentDescription> vulkAttachments(attachments.size());
		std::vector<vk::SubpassDescription> vulkSubpasses(subpasses.size());
		std::vector<vk::SubpassDependency> vulkDependencies(dependencies.size());

		std::vector<std::vector<vk::AttachmentReference>[4]> subpassAttachmentRefData(vulkSubpasses.size());

		for (size_t i = 0; i < vulkAttachments.size(); ++i) vulkAttachments[i]		= FromRHI(attachments[i]);
		for (size_t i = 0; i < vulkDependencies.size(); ++i) vulkDependencies[i]	= FromRHI(dependencies[i]);
		for (size_t i = 0; i < vulkSubpasses.size(); ++i) vulkSubpasses[i] = FromRHI(subpasses[i],
			subpassAttachmentRefData[i][0], subpassAttachmentRefData[i][1], subpassAttachmentRefData[i][2], subpassAttachmentRefData[i][3]);

		// Create the Vulkan Renderpass
		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType			= vk::StructureType::eRenderPassCreateInfo;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(vulkAttachments.size());		// Number of attachments used by this render pass
		renderPassInfo.pAttachments		= vulkAttachments.data();								// Descriptions of the attachments used by the render pass
		
		renderPassInfo.subpassCount		= static_cast<uint32_t>(vulkSubpasses.size());			
		renderPassInfo.pSubpasses		= vulkSubpasses.data();		

		renderPassInfo.dependencyCount	= static_cast<uint32_t>(vulkDependencies.size());	// Number of subpass dependencies
		renderPassInfo.pDependencies	= vulkDependencies.data();								// Subpass dependencies used by the render pass

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

