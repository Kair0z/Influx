#pragma once

#include "VulkanAPI.h"
#include "RHIRenderPass.h"

namespace Influx::Graphics
{
	class VulkanRenderPass final : public RHIRenderPass
	{
		friend class VulkanAPI; // Only the API can construct these...

	public:
		VulkanRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies);
		virtual ~VulkanRenderPass();

		static vk::SubpassDescription FromRHI(const RHIRenderSubPassDesc& rhiSubpass);
		static vk::AttachmentDescription FromRHI(const RHIRenderPassAttachmentDesc& rhiAttachment);
		static vk::SubpassDependency FromRHI(const RHIRenderSubPassDependency& rhiDependency);
		static vk::RenderPass FromRHI(const vk::Device& device, const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies);

		const vk::RenderPass& GetVulkanRenderPass() const;

		virtual void OnUpdateRHI(GraphicsAPI* api) override final;
		void ReconstructVulkanRenderPass(const VulkanAPI* api);

	private:
		vk::RenderPass VulkRenderPass;
	};
}


