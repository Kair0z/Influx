#pragma once

#include "Runtime/RHI/CommandList.h"
#include "D3D12API.h"

#include "Core/Memory/Reference.h"

namespace Influx
{
	class RHIRenderTarget;

	class D3D12GraphicsCommandList final : public RHIGraphicsCommandList
	{
	public:
		virtual void SetPipeline(Ptr<RHIGraphicsPipeline> pipeline) const override;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) const override;
		//virtual void SetVertexBuffer(Ptr<Buffer> vertexBuffer) const override;
		//virtual void SetIndexBuffer(Ptr<Buffer> indexBuffer) const override;
		//virtual void UploadBufferData(Ptr<Buffer> target, void* newData, uint32_t numElements, uint32_t elementSize) override;
		virtual void CopyRenderTarget(Ptr<RHIRenderTarget> source, Ptr<RHIRenderTarget> dest) const override;
		virtual void DrawIndexedInstanced(uint32_t iCountPerInstance) const override;
		virtual void Set32BitConstants(uint32_t bindingSlot, uint32_t num32Values, const void* data) const override;
		virtual void SetRenderTarget(const Ptr<RHIRenderTarget> renderTarget, const Ptr<RHIRenderTarget> depthStencilTarget = nullptr) const override;
		virtual void ClearRenderTarget(const Ptr<RHIRenderTarget> renderTarget, const Math::Vector4f& clearColor) const override;
		virtual void ClearDepthStencil(const Ptr<RHIRenderTarget> renderTarget, const float depthClear, const float stencilClear = 0) const override;
		virtual void SetViewport(const Math::Rectf& viewportRect) const override;
		virtual void SetScissorRects(const Math::Rectf& scissorRect) const override;
		virtual void Close() const override;

		Ptr<ID3D12GraphicsCommandList> GetD3D12CommandList();

		D3D12GraphicsCommandList(Ptr<ID3D12GraphicsCommandList> d3d12CommandList);
		D3D12GraphicsCommandList(const D3D12GraphicsCommandList&) = delete;
		D3D12GraphicsCommandList(D3D12GraphicsCommandList&&) = delete;
		D3D12GraphicsCommandList& operator=(const D3D12GraphicsCommandList&) = delete;
		D3D12GraphicsCommandList& operator=(D3D12GraphicsCommandList&&) = delete;
		~D3D12GraphicsCommandList();

	private:
		Ptr<ID3D12GraphicsCommandList> mpD3D12CommandList;
	};
}


