#pragma once

#include "GraphicsAPI.h"

namespace Influx::Graphics
{
	/* GPU Resource */
	class RHIResource
	{
	public:
		ERHIResourceState GetCurrentState() const { return CurrentState; }
		ERHIResourceState GetPreviousState() const { return PreviousState; }
		void Transition(ERHIResourceState newState) { PreviousState = CurrentState; CurrentState = newState; }

		RHIResource() = default;
		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
		virtual ~RHIResource() = default;

	protected:
		ERHIResourceState PreviousState;
		ERHIResourceState CurrentState;
	};

	/* Texture */
	struct RHITextureDescription
	{
		float Width;
		float Height;

		Math::Vector4f OptimizedClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

		ERHIFormat Format = ERHIFormat::RGBA_8_Unorm;
		UINT16 MipLevels = 1;

		ERHIResourceState InitialResourceState = ERHIResourceState::RenderTarget;
	};
	class RHITexture
	{
	public:
		RHIResource* GetRHIResource() const { return Resource; }
		RHIRenderTargetView* GetRenderTargetView() const { return RenderTargetView; }

		float GetWidth() const { return ConstructionDescription.Width; }
		float GetHeight() const { return ConstructionDescription.Height; }
		ERHIFormat GetRHIFormat() const { return ConstructionDescription.Format; }
		UINT16 GetMipLevels() const { return ConstructionDescription.MipLevels; }
		const Math::Vector4f& GetOptimizedClearValue() const { return ConstructionDescription.OptimizedClearValue; }

		RHITexture() = default;
		RHITexture(const RHITexture&) = delete;
		RHITexture(RHITexture&&) = delete;
		RHITexture& operator=(const RHITexture&) = delete;
		RHITexture& operator=(RHITexture&&) = delete;
		virtual ~RHITexture();

	protected:
		RHITextureDescription ConstructionDescription;

		RHIResource* Resource;
		RHIRenderTargetView* RenderTargetView;
	};

	/* Vertex Buffer */
	class RHIVertexBuffer
	{
	public:
		RHIVertexBuffer() = default;
		RHIVertexBuffer(const RHIVertexBuffer&) = delete;
		RHIVertexBuffer(RHIVertexBuffer&&) = delete;
		RHIVertexBuffer& operator=(const RHIVertexBuffer&) = delete;
		RHIVertexBuffer& operator=(RHIVertexBuffer&&) = delete;
		virtual ~RHIVertexBuffer();

	protected:
		RHIResource* GpuResource;
	};

	/* Constant Buffer */
	class RHIConstantBuffer
	{
	public:
		RHIConstantBuffer() = default;
		RHIConstantBuffer(const RHIConstantBuffer&) = delete;
		RHIConstantBuffer(RHIConstantBuffer&&) = delete;
		RHIConstantBuffer& operator=(const RHIConstantBuffer&) = delete;
		RHIConstantBuffer& operator=(RHIConstantBuffer&&) = delete;
		virtual ~RHIConstantBuffer();

	protected:
		RHIResource* GpuResource;
		RHIConstantBufferView* ConstantBufferView;
	};



	/* Render Target View */
	class RHIRenderTargetView
	{
	public:
		RHIRenderTargetView() = default;
		RHIRenderTargetView(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView(RHIRenderTargetView&&) = delete;
		RHIRenderTargetView& operator=(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView& operator=(RHIRenderTargetView&&) = delete;
		virtual ~RHIRenderTargetView() = default;
	};

	/* Constant Buffer View */
	class RHIConstantBufferView
	{
	public:
		RHIConstantBufferView() = default;
		RHIConstantBufferView(const RHIConstantBufferView&) = delete;
		RHIConstantBufferView(RHIConstantBufferView&&) = delete;
		RHIConstantBufferView& operator=(const RHIConstantBufferView&) = delete;
		RHIConstantBufferView& operator=(RHIConstantBufferView&&) = delete;
		virtual ~RHIConstantBufferView() = default;
	};

}


