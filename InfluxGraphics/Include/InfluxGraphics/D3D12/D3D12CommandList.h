#pragma once

#ifndef __GR_D3D12_COMMANDLIST_H_
#define __GR_D3D12_COMMANDLIST_H_

#include "InfluxGraphics/RHICommandList.h"
#include "D3D12.h"

#include "D3D12CommandQueue.h"

namespace Influx::Graphics
{
	/* D3D12CommandList */
	class D3D12CommandList final : public RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList*)>) override final;
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vectorf4& clearValue) override final;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) override final;
		virtual void BindViewports(const RHIViewport& viewport) override final;
		virtual void BindVertexBuffer(RHIResource* vertexBufferResource, uint32 bufferSizeInBytes, uint32 vertexStrideInBytes) override final;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) override final;
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vectorf4& clearValue, bool forceTransition) override final;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) override final;
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) override final;
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) override final;
		virtual void DrawInstanced(uint32 numVerticesPerInstance, uint32 numInstances, uint32 startVertexLocation, uint32 startInstanceLocation) override final;
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) override final;

		ID3D12GraphicsCommandList* GetDxCommandList() const;

		virtual ~D3D12CommandList();

	private:
		ID3D12GraphicsCommandList* mp_dxCommandList;

		friend class D3D12CommandQueue;
		D3D12CommandList(const ERHICommandQueueType type);
	};
}

#endif