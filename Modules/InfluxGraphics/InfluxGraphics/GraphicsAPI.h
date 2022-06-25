#pragma once

#include <Windows.h>

namespace Influx::Graphics
{
	class RHISwapChain;
	class RHICommandQueue;
	class RHIResource;

#pragma region Enums
	// Types of resources bindable to a Pipeline
	enum class ERHIResourceBindingType
	{
		Constants,
		CBV,
		SRV,
		UAV
	};

	enum class ERHIResourceFlags
	{
		None = 0,
		AllowRenderTarget = 0x1,
		AllowDepthStencil = 0x2,
		AllowUnorderedAccess = 0x4,
		DenyShaderResource = 0x8,
		AllowCrossAdapter = 0x10,
		AllowSimultaneousAccess = 0x20,
		VideoDecodeReferenceOnly = 0x40,
		VideoEncodeReferenceOnly = 0x80
	};

	enum class ERHIResourceState
	{
		Common,
		VertexAndConstantBuffer,
		IndexBuffer,
		RenderTarget,
		UnorderedAccess,
		DepthWrite,
		DepthRead,
		Present,
		RaytracingAS,
		CopyDest,
		CopySource,
		GenericRead,
		AllShaderResource,
		NonPixelReadResource,
		PixelShaderResource,
		Invalid
	};

	// [TODO]
	enum class ERHIShaderStageFlags
	{
		Default
	};

	enum class ERHIFormat
	{
		/* 4 */
		RGBA_32_Float,
		RGBA_8_Unorm,

		/* 3 */
		RGB_32_Float,

		/* 1 */
		R_16_Uint,
		D_32_Float,

		INVALID
	};

	enum class ERHIPrimitiveTopology
	{
		TriangleList
	};

	enum class ERHIPrimitiveTopologyType
	{
		Triangle
	};

	enum class ECommandQueueType
	{
		Graphics
	};
#pragma endregion

	class GraphicsAPI
	{
	public:
		/* Graphics API Interface: */
		virtual RHISwapChain* CreateSwapChain(HWND windowHandle, RHICommandQueue* commandQueue) const = 0;
		virtual RHICommandQueue* CreateCommandQueue(const ECommandQueueType type) const = 0;
		virtual void CreateBuffer() const {};
		virtual void CreateTexture() const {};
		virtual void CreateShader() const {};
		virtual void CreateSampler() const {};
		virtual void CreatePipelineState() const {};
		virtual void CreateRenderPass() const {};
		virtual void CreateRaytracingAccelerationStructure() {};
		virtual void CreateRaytracingPipelineState() {};

		virtual ~GraphicsAPI() = default;
	};

	class RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
		virtual void BeginRenderPass() {};
		virtual void EndRenderPass() {};
		virtual void BindScissorRects() {};
		virtual void BindViewports() {};
		virtual void BindResources() {};
		virtual void BindUAVs() {};
		virtual void BindSampler() {};
		virtual void BindConstantBuffer() {};
		virtual void BindVertexBuffers() {};
		virtual void BindIndexBuffer() {};
		virtual void BindStencilRef() {};
		virtual void BindBlendFactor() {};
		virtual void BindShadingRate() {};
		virtual void BindPipelineState() {};
		virtual void BindComputeShader() {};

		virtual void DrawIndexed() {};
		virtual void DrawInstanced() {};
		virtual void DrawIndexedInstanced() {};
		virtual void DrawInstancedIndirect() {};
		virtual void DrawIndexedInstancedIndirect() {};
		virtual void DrawInstancedIndirectCount() {};
		virtual void DrawIndexedInstancedIndirectCount() {};

		virtual void Dispatch() {};
		virtual void DispatchIndirect() {};
		virtual void CopyResource() {};
		virtual void CopyBuffer() {};
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) {};

		virtual void ClearUAV() {};

		virtual ~RHICommandList() = default;
	};

	class RHICommandQueue
	{
	public:
		virtual RHICommandList* SetupNewCommandList(GraphicsAPI* api) = 0;
		virtual void ExecuteCommmandList(RHICommandList* commandList) = 0;
		virtual void Flush() = 0;

		virtual ~RHICommandQueue() = default;

	protected:
		ECommandQueueType eType;
	};

	class RHIResource
	{
	public:
		ERHIResourceState GetCurrentState() const { return CurrentState; }
		void Transition(ERHIResourceState newState) { PreviousState = CurrentState; CurrentState = newState; }
		virtual ~RHIResource() = default;

	protected:
		ERHIResourceState PreviousState;
		ERHIResourceState CurrentState;
	};

	class RHISwapChain
	{
	public:
		virtual void Present(bool VSync) = 0;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) = 0;
		RHIResource* GetCurrentBackBufferResource() { return BackBufferResources[GetCurrentBackBufferIndex()]; };

		const UINT32 GetCurrentBackBufferIndex() const { return CurrentBackBufferIndex; }
		const UINT32 GetWidth() const { return Width; }
		const UINT32 GetHeight() const { return Height; }

		virtual ~RHISwapChain() = default;

		constexpr static UINT8 NumBackBuffers = 3;

	protected:
		RHIResource* BackBufferResources[NumBackBuffers];

		UINT32 CurrentBackBufferIndex = 0;
		UINT32 Width = 0;
		UINT32 Height = 0;

		bool bIsTearingSupported = false;
	};

	
}


