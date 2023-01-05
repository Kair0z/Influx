#pragma once

#include "GraphicsAPI.h"

namespace Influx::Graphics
{
	/* GPU Resource */
	class RHIResource
	{
	protected:
		friend class GraphicsAPI;

	public:
		ERHIResourceState GetCurrentState() const { return CurrentState; }
		ERHIResourceState GetPreviousState() const { return PreviousState; }
		void Transition(ERHIResourceState newState) { PreviousState = CurrentState; CurrentState = newState; }

		virtual ~RHIResource() = default;

	protected:
		ERHIResourceState PreviousState;
		ERHIResourceState CurrentState;

		RHIResource() = default;
		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
	};

	/* Texture */
	struct RHITextureDescription
	{
		RHITextureDescription() = default;
		RHITextureDescription(uint32_t w, uint32_t h, ERHIFormat format, uint16_t numMips = 1, const Math::Vector4f& optimizedClearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
			ERHIResourceState initialResourceState = ERHIResourceState::RenderTarget);

		uint32_t Width		= 1;
		uint32_t Height		= 1;

		Math::Vector4f OptimizedClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

		ERHIFormat Format;
		uint16_t MipLevels	= 1;

		ERHIResourceState InitialResourceState;
	};

	class RHITexture
	{
		friend class GraphicsAPI;

	public:
		RHIResource* GetRHIResource() const { return Resource; }
		RHIRenderTargetView* GetRenderTargetView() const { return RenderTargetView; }

		uint32_t GetWidth() const { return ConstructionDescription.Width; }
		uint32_t GetHeight() const { return ConstructionDescription.Height; }
		ERHIFormat GetRHIFormat() const { return ConstructionDescription.Format; }
		UINT16 GetMipLevels() const { return ConstructionDescription.MipLevels; }
		const Math::Vector4f& GetOptimizedClearValue() const { return ConstructionDescription.OptimizedClearValue; }

		virtual ~RHITexture();

	protected:
		RHITextureDescription ConstructionDescription;

		RHIResource* Resource;
		RHIRenderTargetView* RenderTargetView;

		RHITexture() = default;
		RHITexture(const RHITexture&) = delete;
		RHITexture(RHITexture&&) = delete;
		RHITexture& operator=(const RHITexture&) = delete;
		RHITexture& operator=(RHITexture&&) = delete;
	};

	/* Vertex Buffer */
	class RHIVertexBuffer
	{
		friend class GraphicsAPI;

	public:
		virtual ~RHIVertexBuffer();

	protected:
		RHIResource* GpuResource;

		RHIVertexBuffer() = default;
		RHIVertexBuffer(const RHIVertexBuffer&) = delete;
		RHIVertexBuffer(RHIVertexBuffer&&) = delete;
		RHIVertexBuffer& operator=(const RHIVertexBuffer&) = delete;
		RHIVertexBuffer& operator=(RHIVertexBuffer&&) = delete;
	};

	/* Constant Buffer */
	class RHIConstantBuffer
	{
		friend class GraphicsAPI;

	public:
		virtual ~RHIConstantBuffer();

	protected:
		RHIResource* GpuResource;
		RHIConstantBufferView* ConstantBufferView;

		RHIConstantBuffer() = default;
		RHIConstantBuffer(const RHIConstantBuffer&) = delete;
		RHIConstantBuffer(RHIConstantBuffer&&) = delete;
		RHIConstantBuffer& operator=(const RHIConstantBuffer&) = delete;
		RHIConstantBuffer& operator=(RHIConstantBuffer&&) = delete;
	};

	/* Render Target View */
	class RHIRenderTargetView
	{
		friend class GraphicsAPI;

	public:
		virtual ~RHIRenderTargetView() = default;

	protected:
		RHIRenderTargetView() = default;
		RHIRenderTargetView(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView(RHIRenderTargetView&&) = delete;
		RHIRenderTargetView& operator=(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView& operator=(RHIRenderTargetView&&) = delete;
	};

	/* Constant Buffer View */
	class RHIConstantBufferView
	{
		friend class GraphicsAPI;

	public:
		virtual ~RHIConstantBufferView() = default;

	protected:
		RHIConstantBufferView() = default;
		RHIConstantBufferView(const RHIConstantBufferView&) = delete;
		RHIConstantBufferView(RHIConstantBufferView&&) = delete;
		RHIConstantBufferView& operator=(const RHIConstantBufferView&) = delete;
		RHIConstantBufferView& operator=(RHIConstantBufferView&&) = delete;
	};
}


