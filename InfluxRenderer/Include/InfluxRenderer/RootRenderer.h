#pragma once

#ifndef __RENDERER_ROOT_RENDERER_H_
#define __RENDERER_ROOT_RENDERER_H_

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"
#include "Core/Container/List.h"
#include "Core/Platform/Platform.h"
#include "Core/Cache.h"
#include "Core/Function.h"

#include "InfluxGraphics/RHITypes.h"

#include "InfluxGraphics/RHIPipeline.h"
#include "InfluxGraphics/RHIPipelineLayout.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHICommandList;
	class RHICommandQueue;
	class RHIDescriptorHeap;
	class RHISwapchain;
	class RHIGraphicsPipeline;
	class RHIGraphicsPipelineLayout;
	class RHIResource;
	class RHITexture;
	struct RHITextureDesc;
	struct RHIGraphicsPipelineDescription;
	struct RHIGraphicsPipelineLayoutDescription;
}

namespace Influx::Renderer
{
	/* 
	* Root Renderer
	* 
	*/
	class RootRenderer final
	{
	public:
#pragma region TypeAliases
		using Ptr = RootRenderer*;
		using SwapchainPtr = Graphics::RHISwapchain*;
		using GraphicsPipelinePtr = Graphics::RHIGraphicsPipeline*;
		using GraphicsPipelineLayoutPtr = Graphics::RHIGraphicsPipelineLayout*;
		using TexturePtr = Graphics::RHITexture*;

		using GfxPipelineKey = Graphics::RHIGraphicsPipelineDescription;
		using GfxPipelineLayoutKey = Graphics::RHIGraphicsPipelineLayoutDescription;

		using TextureCache = Cache<TexturePtr, String>;
		using GfxPipelineCache = Cache<GraphicsPipelinePtr, GfxPipelineKey, GfxPipelineLayoutKey>;
		using GfxPipelineLayoutCache = Cache<GraphicsPipelineLayoutPtr, GfxPipelineLayoutKey>;

		using OnBuildCommandList = Function<void(Graphics::RHICommandList*)>;
		typedef void (*OnWindowResize)(const Math::Vectoru2& newSize);
#pragma endregion

		RootRenderer(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);
		static Ptr Create(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);
		static void Destroy(Ptr& renderer);
		virtual ~RootRenderer();

		/* Runs Command Queues */
		void Render();

		/* Runs Command Queues and schedules the passed CommandList to be built */
		void Render(OnBuildCommandList internalRenderClb);

		/* Present Swapchain */
		void Present(bool vsync);

		/* Attach to Window and create a Swapchain */
		bool AttachToWindow(Platform::WindowHandle windowHandle);
		bool DetachFromCurrentWindow();
		bool IsAttachedToWindow() const;

		/* Signal Window to be resized */
		bool SignalWindowResize(const Math::Vectoru2& newSize);
		bool DoesSwapchainNeedResize() const;

		/* Set Dynamic Graphics API */
		void SetGraphicsAPI(const Graphics::EGraphicsAPI api);
		Graphics::EGraphicsAPI GetCurrentGraphicsAPI() const;
		static bool IsGraphicsAPISupported(const Graphics::EGraphicsAPI api);

		/* Creating Graphics PSO */
		GraphicsPipelinePtr GetAndOrCreateGraphicsPipeline(const GfxPipelineKey& key, const GfxPipelineLayoutKey& pipelineLayoutKey);
		GraphicsPipelineLayoutPtr GetAndOrCreateGraphicsPipelineLayout(const GfxPipelineLayoutKey& key);
		
		/* Creating Textures */
		TexturePtr GetAndOrCreateTexture(const String& key, const Graphics::RHITextureDesc& desc);

		SwapchainPtr GetWindowSwapchain() const;
		const Math::Vectoru2& GetWindowSwapchainDimensions() const;

		uint64 GetFrame() const;

		Graphics::RHIDevice* GetDevice() const;

	private:
		/* RHI Graphics Device */
		Graphics::RHIDevice* mp_rhiDevice;
		Graphics::EGraphicsAPI m_currentGraphicsAPI = Graphics::EGraphicsAPI::NotSupported;
		Graphics::EGraphicsAPI m_initializedDeviceAPI = Graphics::EGraphicsAPI::NotSupported;

		Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
		Graphics::RHICommandQueue* mp_gfxCommandQueue;
		Graphics::RHICommandList* mp_commandList;

		/* Cached Graphics Pipelines */
		GfxPipelineCache mp_graphicsPipelineCache;
		GfxPipelineLayoutCache mp_graphicsPipelineLayoutCache;
		
		/* Cached Textures */
		TextureCache mp_textureCache;

		/* Swapchain */
		struct SwapchainTarget final
		{
			Graphics::RHISwapchain* mp_rhiSwapchain;
			Math::Vectoru2 m_previousSize;
			Math::Vectoru2 m_updatedSize;
			bool m_isDirty = true;
		};
		SwapchainTarget* mp_windowSwapchain = nullptr;

		uint64 m_frame;

		/* Is Renderer initialized on this specific EGraphicsAPI? */
		bool IsGraphicsAPIInitialized(const Graphics::EGraphicsAPI api) const;
		bool IsGraphicsAPIInitialized() const;

		/* Initializes GraphicsDevice object */
		void InitializeGraphicsAPI(const Graphics::EGraphicsAPI api);

		/* Cleans up an initialized GraphicsDevice object */
		void CleanupGraphicsAPI(const Graphics::EGraphicsAPI api);
		void Cleanup();

		void UpdateSwapchain();
	};
}

#endif