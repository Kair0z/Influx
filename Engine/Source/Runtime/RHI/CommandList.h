#pragma once

#ifndef _GFX_COMMAND_LIST_H_
#define _GFX_COMMAND_LIST_H_

#include "Core/Math/Math.h"
#include "Runtime/RHI/RHITypes.h"
#include "Core/Memory/Reference.h"

namespace Influx
{
	class RHIGraphicsPipeline;
	class RHIRenderTarget;

	/* ONLY functions, zero data */
	class RHIGraphicsCommandList
	{
	public:
		virtual void SetPipeline(Ptr<RHIGraphicsPipeline> pipeline) const = 0;
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) const = 0;
		virtual void SetViewport(const Math::Rectf& ViewportRect) const = 0;
		virtual void SetScissorRects(const Math::Rectf& ScissorRect) const = 0;
		virtual void CopyRenderTarget(Ptr<RHIRenderTarget> source, Ptr<RHIRenderTarget> dest) const = 0;
		/* Sets a Render Target, and an optional DepthStencil Target */
		virtual void SetRenderTarget(Ptr<RHIRenderTarget> RenderTarget, const Ptr<RHIRenderTarget> depthStencilTarget = nullptr) const = 0;
		virtual void ClearRenderTarget(Ptr<RHIRenderTarget> RenderTarget, const Math::Vector4f& ClearColor) const = 0;
		virtual void ClearDepthStencil(Ptr<RHIRenderTarget> RenderTarget, const float DepthClear, const float StencilClear = 0) const = 0;
		virtual void DrawIndexedInstanced(uint32_t iCountPerInstance) const = 0;
		virtual void Set32BitConstants(uint32_t BindingSlot, uint32_t Num32Values, const void* Data) const = 0;
		virtual void Close() const = 0;

		//virtual void SetVertexBuffer(Ptr<Buffer> vertexBuffer) const = 0;
		//virtual void UploadBufferData(Ptr<Buffer> target, void* newData, uint32_t numElements, uint32_t elementSize) = 0;
		//virtual void SetIndexBuffer(Ptr<Buffer> indexBuffer) const = 0;
		
		// virtual void TransitionResource(){}
	};

	class RHIComputeCommandList
	{

	};
}

#endif



