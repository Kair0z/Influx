#include "pch.h"
#include "D3D12CommandList.h"
#include "D3D12Pipeline.h"
#include "D3D12RootSignature.h"
#include "D3D12RenderTarget.h"

#include "Core/Assert/Assert.h"
#include "Core/Type/Type.h"

namespace Influx
{
	D3D12GraphicsCommandList::D3D12GraphicsCommandList(Ptr<ID3D12GraphicsCommandList> d3d12CommandList)
	{
		mpD3D12CommandList = d3d12CommandList;
	}

	void D3D12GraphicsCommandList::SetPipeline(Ptr<RHIGraphicsPipeline> pipeline) const
	{
		Ptr<D3D12GraphicsPipeline> d3d12Pipeline = Cast<D3D12GraphicsPipeline>(pipeline);
		ASSERT(d3d12Pipeline != nullptr);

		/* Set the Root Signature associated to this pipeline object first! */
		mpD3D12CommandList->SetGraphicsRootSignature(d3d12Pipeline->GetRootSignatureRef()->GetD3D12RootSignature());

		mpD3D12CommandList->SetPipelineState(d3d12Pipeline->GetD3D12PipelineState());
	}

	void D3D12GraphicsCommandList::SetPrimitiveTopology(ERHIPrimitiveTopology topology) const
	{
		switch (topology)
		{
		case ERHIPrimitiveTopology::TriangleList:
			mpD3D12CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;

		default:
			mpD3D12CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}

	//void D3D12CommandList::SetVertexBuffer(Ptr<Buffer> vertexBuffer) const
	//{
	//	//mpD3D12CommandList->IASetVertexBuffers(0, 1, &Cast<D3D12VertexBuffer>(vertexBuffer)->GetView());
	//}

	//void D3D12CommandList::SetIndexBuffer(Ptr<Buffer> indexBuffer) const
	//{
	//	//mpD3D12CommandList->IASetIndexBuffer(&Cast<D3D12IndexBuffer>(indexBuffer)->GetView());
	//}

	//void D3D12CommandList::UploadBufferData(Ptr<Buffer> target, void* newData, uint32_t numElements, uint32_t elementSize)
	//{
	//	if (newData)
	//	{
	//		target->UploadData(this, newData, numElements);
	//	}
	//}

	void D3D12GraphicsCommandList::CopyRenderTarget(Ptr<RHIRenderTarget> source, Ptr<RHIRenderTarget> dest) const
	{
		D3D12RenderTarget* dxSource = Cast<D3D12RenderTarget>(source);
		D3D12RenderTarget* dxDest = Cast<D3D12RenderTarget>(dest);

		mpD3D12CommandList->CopyResource(dxDest->GetBufferResource(), dxSource->GetBufferResource());
	}

	void D3D12GraphicsCommandList::DrawIndexedInstanced(uint32_t iCountPerInstance) const
	{
		mpD3D12CommandList->DrawIndexedInstanced(iCountPerInstance, 0, 0, 0, 0);
	}

	void D3D12GraphicsCommandList::Set32BitConstants(uint32_t bindingSlot, uint32_t num32Values, const void* data) const
	{
		mpD3D12CommandList->SetGraphicsRoot32BitConstants(bindingSlot, num32Values, data, 0);
	}

	void D3D12GraphicsCommandList::SetRenderTarget(const Ptr<RHIRenderTarget> renderTarget, const Ptr<RHIRenderTarget> depthStencilTarget) const
	{
		D3D12RenderTarget* d3d12Rt = Cast<D3D12RenderTarget>(renderTarget);
		D3D12RenderTarget* d3D12Ds = Cast<D3D12RenderTarget>(depthStencilTarget);

		mpD3D12CommandList->OMSetRenderTargets(1, &d3d12Rt->GetViewCPUHandle(), TRUE, 
			(depthStencilTarget) ? &d3D12Ds->GetViewCPUHandle() : nullptr);
	}

	void D3D12GraphicsCommandList::ClearRenderTarget(const Ptr<RHIRenderTarget> renderTarget, const Math::Vector4f& clearColor) const
	{
		FLOAT clear[] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
		mpD3D12CommandList->ClearRenderTargetView(Cast<D3D12RenderTarget>(renderTarget)->GetViewCPUHandle(), 
			clear, 0, nullptr);
	}

	void D3D12GraphicsCommandList::ClearDepthStencil(const Ptr<RHIRenderTarget> renderTarget, const float depthClear, const float stencilClear) const
	{
		mpD3D12CommandList->ClearDepthStencilView(Cast<D3D12RenderTarget>(renderTarget)->GetViewCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, depthClear, stencilClear, 0, nullptr);
	}

	void D3D12GraphicsCommandList::SetViewport(const Math::Rectf& viewportRect) const
	{
		auto viewport = CD3DX12_VIEWPORT(viewportRect.LB.x,viewportRect.LB.y,
			viewportRect.WH.x, viewportRect.WH.y);

		mpD3D12CommandList->RSSetViewports(1, &viewport);
	}

	void D3D12GraphicsCommandList::SetScissorRects(const Math::Rectf& scissorRect) const
	{
		auto rect = CD3DX12_RECT((LONG)scissorRect.LB.x, (LONG)scissorRect.LB.y, (LONG)scissorRect.WH.x, (LONG)scissorRect.WH.y);
		mpD3D12CommandList->RSSetScissorRects(1, &rect);
	}

	void D3D12GraphicsCommandList::Close() const
	{
		mpD3D12CommandList->Close();
	}

	Ptr<ID3D12GraphicsCommandList> D3D12GraphicsCommandList::GetD3D12CommandList()
	{
		return mpD3D12CommandList;
	}

	D3D12GraphicsCommandList::~D3D12GraphicsCommandList()
	{

	}
}

