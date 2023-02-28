#pragma once

#ifndef __GR_RHI_COMMANDLIST_H_
#define __GR_RHI_COMMANDLIST_H_

#include "Types.h"
#include "RHITypes.h"

namespace Influx::Graphics
{
	class RHIRenderPass;
	struct RHIRenderPassBeginInfo;
	class RHIVertexBuffer;
	class RHIRenderTargetView;
	class RHIGraphicsPipelineLayout;
	class RHIGraphicsPipeline;
	class RHITexture;
	class RHIResource;
	class RHIDescriptorHeap;

	/* Command List */
	class RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList*)>) = 0;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) = 0;
		virtual void BindViewports(const RHIViewport& viewport) = 0;
		virtual void BindResources() {};
		virtual void BindUAVs() {};
		virtual void BindSampler() {};
		virtual void BindConstantBuffer() {};
		virtual void BindVertexBuffer(RHIResource* vertexBufferResource, uint32 bufferSizeInBytes, uint32 vertexStrideInBytes) = 0;
		virtual void BindIndexBuffer(RHIResource* indexBufferResource, uint32 bufferSizeInBytes) = 0;
		virtual void BindStencilRef() {};
		virtual void BindBlendFactor() {};
		virtual void BindShadingRate() {};
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) = 0;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) = 0;
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) = 0;
		virtual void BindComputeShader() {};
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) = 0;

		virtual void DrawIndexed() {};
		virtual void DrawInstanced(uint32 numVerticesPerInstance, uint32 numInstances, uint32 startVertexLocation = 0, uint32 startInstanceLocation = 0) = 0;
		virtual void DrawIndexedInstanced(uint32 numIndicesPerInstance, uint32 numInstances, uint32 startIndexLocation, uint32 startVertexLocation, uint32 startInstanceLocation) = 0;
		virtual void DrawInstancedIndirect() {};
		virtual void DrawIndexedInstancedIndirect() {};
		virtual void DrawInstancedIndirectCount() {};
		virtual void DrawIndexedInstancedIndirectCount() {};

		virtual void Dispatch() {};
		virtual void DispatchIndirect() {};
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition = true) = 0;
		virtual void CopyBuffer() {};
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) = 0;

		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) = 0;
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vectorf4& clearValue, bool forceTransition) = 0;
		virtual void ClearUAV() {};
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView) = 0;
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vectorf4& clearValue) = 0;

		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) = 0;

		ERHICommandQueueType GetType() const;

	protected:
		RHICommandList(const ERHICommandQueueType type);

		RHICommandList(const RHICommandList&) = delete;
		RHICommandList(RHICommandList&&) = delete;
		RHICommandList& operator=(const RHICommandList&) = delete;
		RHICommandList& operator=(RHICommandList&&) = delete;
		virtual ~RHICommandList() = default;

	private:
		ERHICommandQueueType m_type;
	};
}

#endif