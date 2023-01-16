#pragma once

#ifndef __RENDERER_RENDERER_H_
#define __RENDERER_RENDERER_H_

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"
#include "Core/Container/List.h"
#include "Core/Platform/Platform.h"

#include "InfluxGraphics/RHITypes.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHICommandList;
	class RHICommandQueue;
	class RHIDescriptorHeap;
	class RHISwapchain;
}

namespace Influx::Renderer
{
	class IRenderer
	{
	protected:
		using RHIDevicePtr		= Ptr<Influx::Graphics::RHIDevice>;
		using RHICommandListPtr = Ptr<Influx::Graphics::RHICommandList>;
		using RHISwapchainPtr	= Ptr<Influx::Graphics::RHISwapchain>;

	public:
		/* Submitting work onto a passed RHICommandList */
		virtual void OnRender(RHICommandListPtr commandList) const = 0;

		/* On attaching to a Window */
		virtual void OnAttachToWindow(const RHIDevicePtr, const RHISwapchainPtr) {};

		/* Submitting work onto a passed RHICommandList  */
		/* Resizing the bound window swapchain */
		virtual void OnSwapchainResize(const RHIDevicePtr, const RHISwapchainPtr, 
			const Math::Vectoru2& prevSize, const Math::Vectoru2& newSize) {};

		/* Initializing RHI Resources */
		virtual void Initialize(const RHIDevicePtr) = 0;

		/* On detaching from a Window */
		virtual void OnDetachFromWindow(const RHIDevicePtr) {};

		/* Cleaning up RHI Resources */
		virtual void Cleanup(const RHIDevicePtr) = 0;

		bool IsInitialized() const;
		bool NeedsSwapchainUpdate() const;
		bool IsAttachedToSwapchain() const;

	protected:
		IRenderer() = default;

	private:
		bool m_isInitialized = false;
		bool m_hasOutdatedSwapchain = true;
		bool m_isAttachedToSwapchain = false;

		friend class RootRenderer;

	public:
		IRenderer(const IRenderer&) = delete;
		IRenderer(IRenderer&&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;
		IRenderer& operator=(IRenderer&&) = delete;
		virtual ~IRenderer() = default;
	};

	class RootRenderer final
	{
		using IRendererList = List<IRenderer*>;
		IRendererList mp_childRenderers;

	public:
		typedef void (*OnRenderClb)(Graphics::RHICommandList* cmdList);

		void Render();
		void Render(OnRenderClb internalRenderClb);
		void Present(bool vsync);

		void Initialize(const Graphics::EGraphicsAPI api);

		bool AttachToWindow(Platform::WindowHandle windowHandle);
		bool DetachFromCurrentWindow();

		bool OnWindowResize(const Math::Vectoru2& newSize);
		bool DoesSwapchainNeedResize() const;

		bool IsAttachedToWindow() const;

		void SetGraphicsAPI(const Graphics::EGraphicsAPI api);
		Graphics::EGraphicsAPI GetGraphicsAPI() const;

		uint64 GetFrame() const;

		Graphics::RHIDevice* GetDevice() const;

		const IRendererList& GetChildRendererList() const;
		IRendererList& GetChildRendererList();

	public:
		RootRenderer() = default;
		RootRenderer(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);

		virtual ~RootRenderer();

	private:
		/* RHI Graphics Device */
		Graphics::RHIDevice* mp_rhiDevice;
		Graphics::EGraphicsAPI m_currentGraphicsAPI = Graphics::EGraphicsAPI::NotSupported;
		Graphics::EGraphicsAPI m_initializedDeviceAPI = Graphics::EGraphicsAPI::NotSupported;

		Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
		Graphics::RHICommandQueue* mp_gfxCommandQueue;
		Graphics::RHICommandList* mp_commandList;

		struct SwapchainTarget final
		{
			Graphics::RHISwapchain* mp_rhiSwapchain;
			Math::Vectoru2 m_previousSize;
			Math::Vectoru2 m_updatedSize;
			bool m_isDirty = true;
		};
		
		SwapchainTarget* mp_windowSwapchain = nullptr;

		uint64 m_frame;

		void StartRender(Graphics::RHICommandList* cmdList) const;
		void FinishRender(Graphics::RHICommandList* cmdList) const;

		/* Initializes the GraphicsDevice object based on the passed EGraphicsAPI */
		void InitializeDevice(const Graphics::EGraphicsAPI api);

		/* Is Renderer initialized on this specific EGraphicsAPI? */
		bool IsGraphicsInitialized(const Graphics::EGraphicsAPI api) const;

		/* Cleans up currently initialized GraphicsDevice object */
		void Cleanup();

		/* Initialize resources of child renderers */
		void InitializeChildRenderers();
		void AttachChildRenderersToSwapchain();
		void DetachChildRenderersToSwapchain();

		/* Cleanup resources of child renderers */
		void CleanupChildRenderers(bool forceAllCleanup);

		void UpdateSwapchain();
	};
}

#endif
