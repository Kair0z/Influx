#pragma once

#ifndef __GR_D3D12_COMMANDLIST_H_
#define __GR_D3D12_COMMANDLIST_H_

#include "InfluxGraphics/RHICommandList.h"
#include "D3D12.h"

#include "D3D12CommandQueue.h"

namespace influx::Graphics
{
	/* D3D12CommandList */
	class D3D12CommandList final : public RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, function<void(RHICommandList*)>) override final;
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView) override final;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const math::Vectorf4& clearValue) override final;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) override final;
		virtual void BindViewports(const RHIViewport& viewport) override final;
		virtual void BindConstants(const uint32 num32BitConstantsToSet, const float* pData, uint32 rootParameterIndex = 0u) override final;
		virtual void BindConstantBuffer(RHIResource* constantBufferResource, uint32 rootParameterIndex = 0u) override final;
		virtual void BindVertexBuffer(RHIResource* vertexBufferResource, uint64 bufferSizeInBytes, uint64 vertexStrideInBytes) override final;
		virtual void BindIndexBuffer(RHIResource* indexBufferResource, uint64 bufferSizeInBytes) override final;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) override final;
		virtual void SetBlendFactor(const math::Vectorf4& blendFactor) override final;
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, const math::Vectorf4& clearValue, bool forceTransition) override final;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) override final;
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) override final;
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) override final;
		virtual void DrawInstanced(uint32 numVerticesPerInstance, uint32 numInstances, uint32 startVertexLocation, uint32 startInstanceLocation) override final;
		virtual void DrawIndexedInstanced(uint32 numIndicesPerInstance, uint32 numInstances, uint32 startIndexLocation, uint32 startVertexLocation, uint32 startInstanceLocation) override final;
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) override final;

		virtual bool SupportsRenderPasses() const override final;

		ID3D12GraphicsCommandList* GetDxCommandList() const;

		virtual ~D3D12CommandList();

	private:
		ID3D12GraphicsCommandList* mp_dxCommandList;

		friend class D3D12CommandQueue;
		D3D12CommandList(const ERHICommandQueueType type);
	};
}

#endif