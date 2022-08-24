#include "D3D12RenderPass.h"

namespace Influx::Graphics
{
	D3D12RenderPass::D3D12RenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
		const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies)
		: RHIRenderPass(attachments, subpasses, dependencies)
	{

	}

	D3D12RenderPass::~D3D12RenderPass()
	{

	}

	void D3D12RenderPass::OnUpdateRHI(GraphicsAPI* api)
	{

	}
}