#pragma once

#include <Windows.h>
#include "Math/Math.h"
#include <vector>
#include <string>
#include <memory>
#include "Function/Function.h"

namespace Influx::Graphics
{
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
		Undefined,
		Invalid
	};

	enum class ERHIDescriptorType
	{
		Resource,
		DSV,
		RTV,
		Sampler,
		Invalid
	};

	enum class ERHIShaderType
	{
		VertexShader,
		PixelShader
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

	enum class ERHISampleCount
	{
		_1,
		_2,
		_4,
		_8,
		_16,
		_32,
		_64
	};

	enum class ERHIPrimitiveTopology
	{
		TriangleList
	};

	enum class ERHIPrimitiveTopologyType
	{
		Triangle
	};

	enum class ERHICommandQueueType
	{
		Graphics
	};

	enum class ERHICullMode
	{
		None,
		BackFaceCull,
		FrontFaceCull
	};

	enum class ERHIFillMode
	{
		Solid,
		Wireframe
	};

	enum class ERHIPipelineBindPoint
	{
		Graphics
	};

	// Supported Shader Models
	enum class ERHIShaderModel 
	{
		SM_5_0
	};

	// RenderPass
	enum class ERHIRenderPassAttachmentType
	{
		Color
	};

	enum class ERHIRenderPassLoadOp
	{
		Load,			// Preserve the existing contents of the attachment
		Clear,			// Clear the values to a constant at the start
		DontCare		// Existing contents are undefined; we don't care about them
	};

	enum class ERHIRenderPassStoreOp
	{
		Store,			// Rendered contents will be stored in memory and can be read later
		DontCare		// Contents of the framebuffer will be undefined after the rendering operation
	};
#pragma endregion

#pragma region ForwardDeclarations
	class RHISwapChain;
	class RHICommandQueue;
	class RHIResource;
	class RHIRenderTargetView;
	class RHIVertexBuffer;
	class RHITexture;
	struct RHITextureDescription;
	struct RHIScissorRect;
	struct RHIViewport;
	class RHIGraphicsPipelineLayout;
	struct RHIGraphicsPipelineLayoutDescription;
	class RHIGraphicsPipeline;
	struct RHIGraphicsPipelineDescription;
	class RHIShader;
	class RHIConstantBuffer;
	class RHIConstantBufferView;
	class RHIUnorderedAccessView;
	class RHIShaderResourceView;
	class RHIDepthStencilView;
	class RHIDescriptorHeap;
	class RHIRenderPass;
	struct RHIRenderPassBeginInfo;
	struct RHIRenderPassAttachmentDesc;
	struct RHIRenderSubPassDesc;
	struct RHIRenderSubPassDependency;
#pragma endregion

	/* Graphics API */
	class GraphicsAPI
	{
	public:
		/* Graphics API Interface: */
		virtual RHISwapChain* CreateSwapChain(HINSTANCE windowsInstance, HWND windowHandle, RHICommandQueue* commandQueue) const = 0;
		virtual RHICommandQueue* CreateCommandQueue(const ERHICommandQueueType type) const = 0;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const = 0;
		virtual RHIConstantBuffer* CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const = 0;
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const = 0;

		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const = 0;
		virtual RHIRenderTargetView* CreateRenderTargetView(RHIResource* resource) const = 0;
		virtual RHIConstantBufferView* CreateConstantBufferView(RHIResource* resource) const = 0;
		virtual RHIUnorderedAccessView* CreateUnorderedAccessView(RHIResource* resource) const = 0;
		virtual RHIShaderResourceView* CreateShaderResourceView(RHIResource* resource) const = 0;
		virtual RHIDepthStencilView* CreateDepthStencilView(RHIResource* resource) const = 0;

		virtual RHIDescriptorHeap* CreateDescriptorHeap(const ERHIDescriptorType type, uint32_t numDescriptors, bool shaderVisible = false) const = 0;
		virtual RHIGraphicsPipelineLayout* CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const = 0;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const = 0;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& fromCompiledData) const = 0;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const std::string& target) const = 0;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const ERHIShaderType shaderType, const ERHIShaderModel shaderModel = ERHIShaderModel::SM_5_0) const = 0;

		virtual void CreateBuffer() const {};
		virtual void CreateShader() const {};
		virtual void CreateSampler() const {};
		virtual void CreatePipelineState() const {};
		virtual void CreateRaytracingAccelerationStructure() {};
		virtual void CreateRaytracingPipelineState() {};

		virtual RHIRenderPass* CreateRenderPass() const { return nullptr; };
		virtual RHIRenderPass* CreateRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies) const { return nullptr; };

		// Comment: ShaderAPI??
		static std::string MakeShaderTargetString(const ERHIShaderType shaderType, const ERHIShaderModel shaderModel);
		static bool ParseShaderTargetString(const std::string& targetString, ERHIShaderType& outShaderType, ERHIShaderModel& outShaderModel);
		
		virtual ~GraphicsAPI() = default;
	};

	/* Command List */
	class RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList* cmdList)>) = 0;
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) = 0;
		virtual void BindViewports(const RHIViewport& viewport) = 0;
		virtual void BindResources() {};
		virtual void BindUAVs() {};
		virtual void BindSampler() {};
		virtual void BindConstantBuffer() {};
		virtual void BindVertexBuffer(RHIVertexBuffer* vertexBuffer) = 0;
		virtual void BindIndexBuffer() {};
		virtual void BindStencilRef() {};
		virtual void BindBlendFactor() {};
		virtual void BindShadingRate() {};
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) = 0;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) = 0;
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) = 0;
		virtual void BindComputeShader() {};
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) {};

		virtual void DrawIndexed() {};
		virtual void DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0) = 0;
		virtual void DrawIndexedInstanced() {};
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
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition) = 0;
		virtual void ClearUAV() {};
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue) = 0;

		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) = 0;

		RHICommandList() = default;
		RHICommandList(const RHICommandList&) = delete;
		RHICommandList(RHICommandList&&) = delete;
		RHICommandList& operator=(const RHICommandList&) = delete;
		RHICommandList& operator=(RHICommandList&&) = delete;
		virtual ~RHICommandList() = default;
	};

	/* Command Queue */
	class RHICommandQueue
	{
	public:
		virtual RHICommandList* SetupNewCommandList(GraphicsAPI* api) = 0;
		virtual void ExecuteCommmandList(RHICommandList* commandList) = 0;
		virtual void Flush() = 0;

		RHICommandQueue() = default;
		RHICommandQueue(const RHICommandQueue&) = delete;
		RHICommandQueue(RHICommandQueue&&) = delete;
		RHICommandQueue& operator=(const RHICommandQueue&) = delete;
		RHICommandQueue& operator=(RHICommandQueue&&) = delete;
		virtual ~RHICommandQueue() = default;

	protected:
		ERHICommandQueueType eType;
	};

	/* Swapchain */
	class RHISwapChain
	{
	public:
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) = 0;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) = 0;
		RHIResource* GetCurrentBackBufferResource() { return BackBufferResources[GetCurrentBackBufferIndex()]; }
		RHIRenderTargetView* GetCurrentRenderTargetView() { return BackBufferRTVs[GetCurrentBackBufferIndex()]; }
		const UINT32 GetCurrentBackBufferIndex() const { return CurrentBackBufferIndex; }
		const UINT32 GetWidth() const { return Width; }
		const UINT32 GetHeight() const { return Height; }

		RHISwapChain() = default;
		RHISwapChain(const RHISwapChain&) = delete;
		RHISwapChain(RHISwapChain&&) = delete;
		RHISwapChain& operator=(const RHISwapChain&) = delete;
		RHISwapChain& operator=(RHISwapChain&&) = delete;
		virtual ~RHISwapChain();

		constexpr static UINT8 NumBackBuffers = 3;

	protected:
		RHIResource* BackBufferResources[NumBackBuffers];
		RHIRenderTargetView* BackBufferRTVs[NumBackBuffers];

		UINT32 CurrentBackBufferIndex = 0;
		UINT32 Width = 0;
		UINT32 Height = 0;

		bool bIsTearingSupported = false;
	};

	/* DescriptorHeap */
	class RHIDescriptorHeap
	{
	public:
		RHIDescriptorHeap() = default;
		virtual ~RHIDescriptorHeap() = default;
		const ERHIDescriptorType GetType() const;
		bool IsShaderVisible() const;

	protected:
		ERHIDescriptorType HeapType;
		bool bIsShaderVisible;
		size_t NumDescriptors;
	};

	/* Shader-Stuff */
	class RHIShader
	{
	public:
		ERHIShaderType GetType() const { return Type; }

		RHIShader() = default;
		RHIShader(const RHIGraphicsPipeline&) = delete;
		RHIShader(RHIGraphicsPipeline&&) = delete;
		RHIShader& operator=(const RHIShader&) = delete;
		RHIShader& operator=(RHIShader&&) = delete;
		virtual ~RHIShader() = default;

	protected:
		ERHIShaderType Type;
		ERHIShaderModel ShaderModel;
	};

	/* Viewport */
	struct RHIViewport
	{
		RHIViewport() = default;
		RHIViewport(float width, float height, float left = 0.0f, float bottom = 0.0f) 
			: Width{ width }, Height{ height }, Left{ left }, Bottom{ bottom }{}

		float Width;
		float Height;
		float Bottom;
		float Left;
	};

	/* ScissorRect */
	struct RHIScissorRect
	{
		RHIScissorRect() = default;
		RHIScissorRect(float width, float height, float left = 0.0f, float bottom = 0.0f)
			: Width{ width }, Height{ height }, Left{ left }, Bottom{ bottom }{}

		float Width;
		float Height;
		float Bottom;
		float Left;
	};
}


