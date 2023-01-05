#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12CommandList.h"

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
	}
	void D3D12CommandList::ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vectorf4& clearValue)
	{
	}
	void D3D12CommandList::BindScissorRect(const RHIScissorRect& scissorRect)
	{
	}
	void D3D12CommandList::BindViewports(const RHIViewport& viewport)
	{
	}
	void D3D12CommandList::BindVertexBuffer(RHIVertexBuffer* vertexBuffer)
	{
	}
	void D3D12CommandList::SetPrimitiveTopology(ERHIPrimitiveTopology topology)
	{
	}
	void D3D12CommandList::CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition)
	{
	}
	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, bool forceTransition)
	{
	}
	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, const Math::Vectorf4& clearValue, bool forceTransition)
	{
	}
	void D3D12CommandList::BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout)
	{
	}
	void D3D12CommandList::BindPipelineState(RHIGraphicsPipeline* pipeline)
	{
	}
	void D3D12CommandList::BindRenderTarget(RHIRenderTargetView* renderTargetView)
	{
	}
	void D3D12CommandList::DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation)
	{
	}
	void D3D12CommandList::BindDescriptorheap(RHIDescriptorHeap* descriptorHeap)
	{
	}
}

