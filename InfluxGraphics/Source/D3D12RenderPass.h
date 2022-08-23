#pragma once

#include "RHIRenderPass.h"

namespace Influx::Graphics
{
	class D3D12RenderPass final : public RHIRenderPass
	{
	public:
		D3D12RenderPass(std::vector<RHIRenderPassAttachmentDesc> attachments, std::vector<RHIRenderSubPassDesc> subpasses);
		virtual ~D3D12RenderPass();

	};
}


