#pragma once

#include "D3D12API.h"
#include "RHIRenderPass.h"

namespace Influx::Graphics
{
	class D3D12RenderPass final : public RHIRenderPass
	{
		friend class D3D12API; // Only the API can construct these...

	public:
		D3D12RenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies);
		
		D3D12RenderPass(const D3D12RenderPass&) = delete;
		D3D12RenderPass(D3D12RenderPass&&) = delete;
		D3D12RenderPass& operator=(const D3D12RenderPass&) = delete;
		D3D12RenderPass& operator=(D3D12RenderPass&&) = delete;
		virtual ~D3D12RenderPass();

		virtual void OnUpdateRHI(GraphicsAPI* api) override final;

	private:
		// No such thing as an ID3D12RenderPass :p -> Commandlist handles
	};
}


