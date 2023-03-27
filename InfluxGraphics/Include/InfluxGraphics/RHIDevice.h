#pragma once

#ifndef __GR_RHI_DEVICE_H_
#define __GR_RHI_DEVICE_H_

#include "RHITypes.h"
#include "Core/Pointer.h"
#include "Core/Platform/Platform.h"

namespace Influx::Graphics
{
	class RHICommandList;
	class RHIDescriptorHeap;
	class RHISwapchain;
	class RHICommandQueue;
	class RHIRenderTargetView;
	class RHIShaderResourceView;
	class RHIConstantBufferView;
	class RHIResource;
	class RHIGraphicsPipelineLayout;
	struct RHIGraphicsPipelineDescription;
	class RHIGraphicsPipeline;
	class RHITexture;
	
	struct RHITextureDesc;

	class RHIDevice
	{
	protected:
		using CommandQueuePtr				= Ptr<RHICommandQueue>;
		using SwapchainPtr					= Ptr<RHISwapchain>;
		using DescriptorHeapPtr				= Ptr<RHIDescriptorHeap>;
		using RenderTargetViewPtr			= Ptr<RHIRenderTargetView>;
		using ShaderResourceViewPtr			= Ptr<RHIShaderResourceView>;
		using ResourcePtr					= Ptr<RHIResource>;
		using GraphicsPipelineLayoutPtr		= Ptr<RHIGraphicsPipelineLayout>;
		using GraphicsPipelinePtr			= Ptr<RHIGraphicsPipeline>;
		using TexturePtr					= Ptr<RHITexture>;

	public:
		/* Creating API objects & Resources */
		virtual CommandQueuePtr CreateCommandQueue(const ERHICommandQueueType type) const = 0;

		virtual SwapchainPtr CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, CommandQueuePtr commandQueue) const = 0;

		virtual DescriptorHeapPtr CreateDescriptorHeap(const ERHIResourceViewType type, uint32 numDescriptors, bool isShaderVisible) const = 0;

		virtual RenderTargetViewPtr CreateRenderTargetView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const = 0;
		virtual ShaderResourceViewPtr CreateShaderResourceView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const = 0;

		virtual ResourcePtr CreateResource(const ERHIResourceState initialState) const = 0;
		virtual ResourcePtr CreateTextureResource(const ERHIResourceState initialState, const ERHIFormat format, const Math::Vectoru2& dimensions, const uint16 numMips) const = 0;
		virtual ResourcePtr CreateVertexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const = 0;
		virtual ResourcePtr CreateIndexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const = 0;
		virtual ResourcePtr CreateConstantBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const = 0;

		virtual GraphicsPipelineLayoutPtr CreateGraphicsPipelineLayout() const = 0;
		virtual GraphicsPipelinePtr CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& desc, GraphicsPipelineLayoutPtr rootSignature) const = 0;

		virtual bool UploadDataToTexture(byte* pData, TexturePtr texture) const = 0;

		/* Debug Layer*/
		virtual void SetDebugLayerEnabled(bool setDebugLayerEnabled) = 0;
		bool GetIsDebugLayerEnabled() const;

		virtual EGraphicsAPI GetGraphicsAPI() const = 0;

		/* API agnostic RHI implementations */
		virtual TexturePtr CreateTexture(const ERHIResourceState initialState, const RHITextureDesc& desc) const;

	public:
		CommandQueuePtr GetGlobalGraphicsCommandQueue() const;
		CommandQueuePtr GetGlobalComputeCommandQueue() const;

		DescriptorHeapPtr GetRTVDescriptorHeap() const;
		DescriptorHeapPtr GetDSVDescriptorHeap() const;
		DescriptorHeapPtr GetResourceDescriptorHeap() const;
		DescriptorHeapPtr GetSamplerDescriptorHeap() const;

		RHIRenderTargetView* CreateRenderTargetView(const ResourcePtr viewedResource) const;
		RHIShaderResourceView* CreateShaderResourceView(const ResourcePtr viewedResource) const;

		const uint64 GetRTVDescriptorSize() const;
		const uint64 GetDSVDescriptorSize() const;
		const uint64 GetResourceDescriptorSize() const; // CBVs, UAVs, ConstantBuffers...
		const uint64 GetSamplerDescriptorSize() const;
		const uint64 GetDescriptorSize(const ERHIResourceViewType type) const;

	protected:
		RHIDevice() = default;
		RHIDevice(const RHIDevice&) = delete;
		RHIDevice(RHIDevice&&) = delete;
		RHIDevice& operator=(const RHIDevice&) = delete;
		RHIDevice& operator=(RHIDevice&&) = delete;
		virtual ~RHIDevice() = default;

	protected:
		/* RHIDevice Implementation creates the global Device objects AFTER the API gets created */
		void PostInitialize();

		/* RHIDevice Implementation destroys the global Device objects BEFORE the API gets destroyed */
		void PreCleanup();

		bool m_isDebugLayerEnabled = false;

		// Global Command Queues:
		void CreateGlobalQueues();
		RHICommandQueue* mp_graphicsQueue;
		RHICommandQueue* mp_computeQueue;

		// Global Descriptor Heaps:
		void CreateGlobalDescriptorHeaps();
		uint64 m_cachedRtvDescriptorSize = 0;
		uint64 m_cachedDsvDescriptorSize = 0;
		uint64 m_cachedResourceDescriptorSize = 0;
		uint64 m_cachedSamplerDescriptorSize = 0;

		// Global Descriptor Heaps
		RHIDescriptorHeap* mp_RTVDescriptorHeap;
		RHIDescriptorHeap* mp_resourceDescriptorHeap;
		RHIDescriptorHeap* mp_DSVDescriptorheap;
		RHIDescriptorHeap* mp_samplerDescriptorHeap;

		bool m_isInitialized = false;
		bool m_isCleanedUp = false;

		constexpr static uint8 k_maxNumSamplerDescriptorsPerHeap = 16u;
		constexpr static uint8 k_maxNumResourceDescriptorsPerHeap = 64u;
		constexpr static uint8 k_maxNumRtvDescriptorsPerHeap = 64u;
		constexpr static uint8 k_maxNumDsvDescriptorsPerHeap = 64u;
	};
}

#endif


