#pragma once

#ifndef __RENDERER_ROOT_RENDERER_H_
#define __RENDERER_ROOT_RENDERER_H_

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"
#include "Core/Container/List.h"
#include "Core/Platform/Platform.h"
#include "Core/Cache.h"
#include "Core/Function.h"

#include <type_traits>

#include "InfluxGraphics/RHITypes.h"

#include "InfluxRenderer/IRenderer.h"

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
	class RootRenderer;

	class RenderContext final
	{
	public:
		const Graphics::RHIDevice* GetDevice() const;

		/* Creating Textures */
		Graphics::RHITexture* GetAndOrCreateTexture(const String& key, const Graphics::RHITextureDesc& desc) const;

		/* Creating Graphics PSO */
		Graphics::RHIGraphicsPipeline* GetAndOrCreateGraphicsPipeline(const Graphics::RHIGraphicsPipelineDescription& key, const Graphics::RHIGraphicsPipelineLayoutDescription& pipelineLayoutKey) const;
		
		/* Creating Graphics PSO Layout */
		Graphics::RHIGraphicsPipelineLayout* GetAndOrCreateGraphicsPipelineLayout(const Graphics::RHIGraphicsPipelineLayoutDescription& key) const;

		/* Copying a texture into our current Swapchain */
		bool CopyTextureIntoSwapchain(Graphics::RHITexture* texture, Graphics::RHICommandList* cmdList) const;

		/* Accessing the window swapchain */
		Graphics::RHISwapchain* GetSwapchain() const;

	private:
		RootRenderer* mp_rootRendererPtr = nullptr;

		RenderContext() = default;
		friend class RootRenderer;
	};

	/* 
	* Root Renderer
	* 
	*/
	class RootRenderer final
	{
	public:
#pragma region TypeAliases
		using Ptr = RootRenderer*;
		using DevicePtr = Graphics::RHIDevice*;
		using SwapchainPtr = Graphics::RHISwapchain*;
		using GraphicsPipelinePtr = Graphics::RHIGraphicsPipeline*;
		using GraphicsPipelineLayoutPtr = Graphics::RHIGraphicsPipelineLayout*;
		using TexturePtr = Graphics::RHITexture*;

		using GfxPipelineKey = Graphics::RHIGraphicsPipelineDescription;
		using GfxPipelineLayoutKey = Graphics::RHIGraphicsPipelineLayoutDescription;

		using TextureCache = Cache<TexturePtr, String>;
		using GfxPipelineCache = Cache<GraphicsPipelinePtr, GfxPipelineKey, GfxPipelineLayoutKey>;
		using GfxPipelineLayoutCache = Cache<GraphicsPipelineLayoutPtr, GfxPipelineLayoutKey>;
	
		using IRendererList = Vector<IRenderer*>;

		using OnPostInitializeAPI = Function<void(const Graphics::EGraphicsAPI eApi, Graphics::RHIDevice*)>;
		using OnBuildCommandList = Function<void(Graphics::RHICommandList*)>;
		using OnWindowResize = Function<void(const Math::Vectoru2& newSize)>;
		using OnPreCleanupAPI = Function<void(const Graphics::EGraphicsAPI eApi, Graphics::RHIDevice*)>;
#pragma endregion

	public:
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

		/* Adding child IRenderers as callbacks */
		template <class _R, class ...Args>
		_R* AddRenderer(Args&&... args)
		{
			static_assert(std::is_base_of<IRenderer, _R>::value, "_R must derive from IRenderer interface!");
			
			_R* newRenderer = new _R(args...);
			mp_childRenderers.push_back(newRenderer);

			// Initialize to API
			if (IsGraphicsAPIInitialized(GetCurrentGraphicsAPI()))
			{
				mp_childRenderers.back()->OnPostInitializeAPI(GetCurrentGraphicsAPI(), GetDevice());
			}

			return newRenderer;
		}

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
		const RenderContext& GetRenderContext();

	private:
		/* RHI Graphics Device */
		Graphics::RHIDevice* mp_rhiDevice;
		Graphics::EGraphicsAPI m_currentGraphicsAPI = Graphics::EGraphicsAPI::NotSupported;
		Graphics::EGraphicsAPI m_initializedDeviceAPI = Graphics::EGraphicsAPI::NotSupported;

		Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
		Graphics::RHICommandQueue* mp_gfxCommandQueue;
		Graphics::RHICommandList* mp_commandList;

		IRendererList mp_childRenderers;

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

		RenderContext m_renderContext;
	};
}

#endif