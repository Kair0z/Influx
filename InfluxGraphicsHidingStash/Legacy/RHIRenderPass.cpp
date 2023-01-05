#include "RHIRenderPass.h"

namespace Influx::Graphics
{
	RHIRenderPass::RHIRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
		const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies)
		: Attachments{attachments}
		, SubPasses{subpasses}
		, Dependencies{dependencies}
	{
	}

	void RHIRenderPass::PushAttachment(const RHIRenderPassAttachmentDesc& newAttachment, GraphicsAPI* api)
	{
		Attachments.push_back(newAttachment);
		if (api != nullptr) OnUpdateRHI(api);
	}

	void RHIRenderPass::PushSubPass(const RHIRenderSubPassDesc& subPass, GraphicsAPI* api)
	{
		SubPasses.push_back(subPass);
		if (api != nullptr) OnUpdateRHI(api);
	}

	void RHIRenderPass::PushSubPassDependency(const RHIRenderSubPassDependency& dependency, GraphicsAPI* api)
	{
		Dependencies.push_back(dependency);
		if (api != nullptr) OnUpdateRHI(api);
	}

	const std::vector<RHIRenderPassAttachmentDesc>& RHIRenderPass::GetAttachments() const
	{
		return Attachments;
	}
	const std::vector<RHIRenderSubPassDependency>& RHIRenderPass::GetDependencies() const
	{
		return Dependencies;
	}
	const std::vector<RHIRenderSubPassDesc>& RHIRenderPass::GetSubPasses() const
	{
		return SubPasses;
	}
}

