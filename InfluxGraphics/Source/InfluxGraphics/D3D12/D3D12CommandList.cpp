#include "InfluxGraphics/Common.h"

#include "InfluxGraphics/D3D12/D3D12CommandList.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"
#include "InfluxGraphics/D3D12/D3D12Conversion.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"
#include "InfluxGraphics/D3D12/D3D12Pipeline.h"
#include "InfluxGraphics/D3D12/D3D12PipelineLayout.h"

#include "InfluxGraphics/D3D12/ResourceViews/D3D12RenderTargetView.h"

namespace Influx::Graphics
{
	D3D12CommandList::D3D12CommandList(const ERHICommandQueueType type)
		: RHICommandList(type)
	{

	}

	D3D12CommandList::~D3D12CommandList()
	{
		// ...
	}

	ID3D12GraphicsCommandList* D3D12CommandList::GetDxCommandList() const
	{
		return mp_dxCommandList;
	}
	void D3D12CommandList::RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList*)>)
	{
	
	}

	void D3D12CommandList::TransitionResource(RHIResource* resource, const ERHIResourceState newState)
	{
		if (resource->GetCurrentState() == newState)
		{
			return;
		}

		D3D12Resource* d3d12Resource = (D3D12Resource*)resource;
		ID3D12Resource* dxResource = d3d12Resource->GetDxResource();

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = dxResource;
		barrier.Transition.StateBefore = Conversion::ToDx12(resource->GetCurrentState());
		barrier.Transition.StateAfter = Conversion::ToDx12(newState);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		GetDxCommandList()->ResourceBarrier(1, &barrier);

		// Keep the RHI up to date...
		resource->TransitionState(newState);
	}

	void D3D12CommandList::ClearRTV(RHIRenderTargetView* renderTargetView)
	{
		ClearRTV(renderTargetView, renderTargetView->GetOptimizedClearValue().Colour);
	}

	void D3D12CommandList::ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vectorf4& clearValue)
	{
		D3D12RenderTargetView* dxRtv = (D3D12RenderTargetView*)renderTargetView;

		const FLOAT color[4] = { clearValue[0], clearValue[1], clearValue[2], clearValue[3] };
		GetDxCommandList()->ClearRenderTargetView(dxRtv->GetDxCPUHandle(), color, 0u, nullptr);
	}

	void D3D12CommandList::BindScissorRect(const RHIScissorRect& scissorRect)
	{
		D3D12_RECT rect{scissorRect.Left, scissorRect.Bottom, scissorRect.Width, scissorRect.Height};
		GetDxCommandList()->RSSetScissorRects(1u, &rect);
	}

	void D3D12CommandList::BindViewports(const RHIViewport& viewport)
	{
		D3D12_VIEWPORT d3d12Viewport{viewport.Left, viewport.Bottom, viewport.Width, viewport.Height};
		GetDxCommandList()->RSSetViewports(1u, &d3d12Viewport);
	}

	void D3D12CommandList::BindVertexBuffer(RHIVertexBuffer* vertexBuffer)
	{

	}

	void D3D12CommandList::SetPrimitiveTopology(ERHIPrimitiveTopology topology)
	{
		GetDxCommandList()->IASetPrimitiveTopology(Conversion::ToDx12(topology));
	}

	void D3D12CommandList::CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition)
	{
		TransitionResource(source, ERHIResourceState::CopySource);
		TransitionResource(dest, ERHIResourceState::CopyDest);

		D3D12Resource* d3d12Dest	= (D3D12Resource*)dest;
		D3D12Resource* d3d12Src		= (D3D12Resource*)source;

		GetDxCommandList()->CopyResource(d3d12Dest->GetDxResource(), d3d12Src->GetDxResource());

		TransitionResource(source, source->GetPreviousState());
		TransitionResource(dest, dest->GetPreviousState());
	}

	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, bool forceTransition)
	{
	}
	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, const Math::Vectorf4& clearValue, bool forceTransition)
	{
	}

	void D3D12CommandList::BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout)
	{
		D3D12GraphicsPipelineLayout* d3d12Layout = (D3D12GraphicsPipelineLayout*)pipelineLayout;
		GetDxCommandList()->SetGraphicsRootSignature(d3d12Layout->GetDxRootSignature());
	}

	void D3D12CommandList::BindPipelineState(RHIGraphicsPipeline* pipeline)
	{
	}

	void D3D12CommandList::BindRenderTarget(RHIRenderTargetView* renderTargetView)
	{
		D3D12RenderTargetView* dxRtv = (D3D12RenderTargetView*)renderTargetView;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = dxRtv->GetDxCPUHandle();
		GetDxCommandList()->OMSetRenderTargets(1u, &cpuHandle, TRUE, nullptr);
	}

	void D3D12CommandList::DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation)
	{
	}

	void D3D12CommandList::BindDescriptorheap(RHIDescriptorHeap* descriptorHeap)
	{
		D3D12DescriptorHeap* d3d12DescriptorHeap = (D3D12DescriptorHeap*)descriptorHeap;

		ID3D12DescriptorHeap* dxDescriptorHeaps[]{ d3d12DescriptorHeap->GetDxDescriptorHeap() };
		GetDxCommandList()->SetDescriptorHeaps(1u, dxDescriptorHeaps);
	}
}

