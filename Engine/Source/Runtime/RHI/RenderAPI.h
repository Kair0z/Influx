#pragma once

#ifndef _RENDER_API_H_
#define _RENDER_API_H_

#include "Core/Type/String.h"
#include "Core/Memory/Reference.h"
#include "RHITypes.h"

namespace Influx
{
#pragma region ForwardDeclarations
	class RHIGraphicsPipeline;
	class RHICommandQueue;
	class RHIRenderTarget;
	class RHISwapChain;
	struct CommandQueueDesc;
	struct SwapChainDesc;
	struct GraphicsPipelineBuilder;
#pragma endregion

	/* [Interface] for exposing creating RHIResources */
	class RenderAPI
	{
	public:
		static Ptr<RenderAPI> Create();
		virtual void Initialize() = 0;

		// Creating RHI Resources:
		virtual Ptr<RHIGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineBuilder& desc) = 0;
		virtual Ptr<RHISwapChain> CreateSwapChain(void* windowHandle, const Ptr<RHICommandQueue> commandQueue) = 0;
		virtual Ptr<RHICommandQueue> CreateCommandQueue(const CommandQueueDesc& desc) = 0;
		virtual Ptr<RHIRenderTarget> CreateRenderTarget(const Vector2u& dimensions, const ERHIFormat format) = 0;
		virtual Ptr<RHIRenderTarget> CreateDepthStencilTarget(const Vector2u& dimensions, const ERHIFormat format) = 0;
		//virtual Ptr<Buffer> CreateBuffer(const Buffer::Initializer& init) = 0;
		//virtual Ptr<Buffer> CreateVertexBuffer(const Buffer::Initializer& init) = 0;
		//virtual Ptr<Buffer> CreateIndexBuffer(const Buffer::Initializer& init) = 0;
		//virtual Ptr<Shader> CreateVertexShader(const String& filepath) = 0;
		//virtual Ptr<Shader> CreatePixelShader(const String& filepath) = 0;

		// Misc...:
		virtual void SetupDebugLayer() = 0;

		RenderAPI(const RenderAPI&) = default;
		RenderAPI(RenderAPI&&) = default;
		RenderAPI& operator=(const RenderAPI&) = default;
		RenderAPI& operator=(RenderAPI&&) = default;
		virtual ~RenderAPI() = default;

	protected:
		RenderAPI() = default;
	};
}

#endif


