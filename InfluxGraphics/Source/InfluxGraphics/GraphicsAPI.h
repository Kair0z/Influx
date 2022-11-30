#pragma once

#include <Windows.h>
#include "Math/Math.h"
#include <vector>
#include <string>
#include <memory>
#include "Function/Function.h"

#include "GraphicsTypes.h"

namespace Influx::Graphics
{
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
	class RHICommandList;
	class RHIShader;
#pragma endregion

	/* Graphics API */
	class GraphicsAPI
	{
	public:
		/* Graphics API Interface: */
		/* Creating Resources & RHI Classes */
#pragma region Graphics API Interface
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
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference, RHIRenderPass* renderPass) const = 0;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& compiledData, ERHIShaderType shaderType, ERHIShaderModel shaderModel) const = 0;

		virtual void CreateBuffer() const {};
		virtual void CreateShader() const {};
		virtual void CreateSampler() const {};
		virtual void CreatePipelineState() const {};
		virtual void CreateRaytracingAccelerationStructure() {};
		virtual void CreateRaytracingPipelineState() {};

		virtual RHIRenderPass* CreateRenderPass() const { return nullptr; };
		virtual RHIRenderPass* CreateRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies) const { return nullptr; };
	
		virtual void EnableDebugLayer(bool enable) const {};
#pragma endregion

		/* Graphics Command List Indirect Interface */
#pragma region RHICommandList Indirect Interface
		virtual void CmdRecordRenderPass(RHICommandList* cmdList, RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList* cmdList)>) {};
		virtual void CmdBindScissorRect(RHICommandList* cmdList, const RHIScissorRect& scissorRect) {};
		virtual void CmdBindViewports(RHICommandList* cmdList, const RHIViewport& viewport) {};
		virtual void CmdBindResources(RHICommandList* cmdList) {};
		virtual void CmdBindUAVs(RHICommandList* cmdList) {};
		virtual void CmdBindSampler(RHICommandList* cmdList) {};
		virtual void CmdBindConstantBuffer(RHICommandList* cmdList) {};
		virtual void CmdBindVertexBuffer(RHICommandList* cmdList, RHIVertexBuffer* vertexBuffer) {};
		virtual void CmdBindIndexBuffer(RHICommandList* cmdList) {};
		virtual void CmdBindStencilRef(RHICommandList* cmdList) {};
		virtual void CmdBindBlendFactor(RHICommandList* cmdList) {};
		virtual void CmdBindShadingRate(RHICommandList* cmdList) {};
		virtual void CmdBindRenderTarget(RHICommandList* cmdList, RHIRenderTargetView* renderTargetView) {};
		virtual void CmdBindPipelineLayout(RHICommandList* cmdList, RHIGraphicsPipelineLayout* pipelineLayout) {};
		virtual void CmdBindPipelineState(RHICommandList* cmdList, RHIGraphicsPipeline* pipeline) {};
		virtual void CmdBindComputeShader(RHICommandList* cmdList) {};
		virtual void CmdBindDescriptorheap(RHICommandList* cmdList, RHIDescriptorHeap* descriptorHeap) {};
		virtual void CmdDrawIndexed(RHICommandList* cmdList) {};
		virtual void CmdDrawInstanced(RHICommandList* cmdList, uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0) {};
		virtual void CmdDrawIndexedInstanced(RHICommandList* cmdList) {};
		virtual void CmdDrawInstancedIndirect(RHICommandList* cmdList) {};
		virtual void CmdDrawIndexedInstancedIndirect(RHICommandList* cmdList) {};
		virtual void CmdDrawInstancedIndirectCount(RHICommandList* cmdList) {};
		virtual void CmdDrawIndexedInstancedIndirectCount(RHICommandList* cmdList) {};
		virtual void CmdDispatch(RHICommandList* cmdList) {};
		virtual void CmdDispatchIndirect(RHICommandList* cmdList) {};
		virtual void CmdCopyResource(RHICommandList* cmdList, RHIResource* source, RHIResource* dest, bool forceTransition = true) {};
		virtual void CmdCopyBuffer(RHICommandList* cmdList) {};
		virtual void CmdTransitionResource(RHICommandList* cmdList, RHIResource* resource, const ERHIResourceState newState) {};
		virtual void CmdClearTextureAsRTV(RHICommandList* cmdList, RHITexture* texture, bool forceTransition) {};
		virtual void CmdClearTextureAsRTV(RHICommandList* cmdList, RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition) {};
		virtual void CmdClearUAV(RHICommandList* cmdList) {};
		virtual void CmdClearRTV(RHICommandList* cmdList, RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue) {};
		virtual void CmdSetPrimitiveTopology(RHICommandList* cmdList, ERHIPrimitiveTopology topology) {};
#pragma endregion

		// Comment: ShaderAPI??
		static std::string MakeShaderTargetString(const ERHIShaderType shaderType, const ERHIShaderModel shaderModel);
		static bool ParseShaderTargetString(const std::string& targetString, ERHIShaderType& outShaderType, ERHIShaderModel& outShaderModel);
		
		GraphicsAPI() = default;
		GraphicsAPI(const GraphicsAPI&) = delete;
		GraphicsAPI(GraphicsAPI&&) = delete;
		GraphicsAPI& operator=(const GraphicsAPI&) = delete;
		GraphicsAPI& operator=(GraphicsAPI&&) = delete;
		virtual ~GraphicsAPI() = default;
	};

	/* Command List */
	class RHICommandList
	{
	public:
		/* Graphics Commandlist Interface: */
#pragma region RHICommandList Interface
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
#pragma endregion

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
		/* Serves a new RHICommandList to record commands. */
		virtual RHICommandList* SetupNewCommandList(GraphicsAPI* api) = 0;

		/* Executes a recorded RHICommandList */
		virtual void ExecuteCommmandList(RHICommandList* commandList) = 0;
		
		/* Flush all GPU work */
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
		/* Flips & Presents the backbuffer to the front-buffer. */
		// Also handles synchronization with the given RHICommandQueue
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) = 0;

		/* Recreates RHISwapchain resources based on the new size */
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
		RHIScissorRect(uint32_t width, uint32_t height, uint32_t left = 0.0f, uint32_t bottom = 0.0f)
			: Width{ width }, Height{ height }, Left{ left }, Bottom{ bottom }{}

		uint32_t Width;
		uint32_t Height;
		uint32_t Bottom;
		uint32_t Left;
	};
}


