#pragma once


#ifndef __RENDERER_ROOT_RENDERER_H_
#define __RENDERER_ROOT_RENDERER_H_

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"
#include "Core/Container/List.h"
#include "Core/Platform/Platform.h"

#include "InfluxGraphics/RHITypes.h"

#include "IRenderer.h"

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
	class RootRenderer final
	{
		using IRendererList = List<IRenderer*>;
		using Ptr = RootRenderer*;

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

		static Ptr Create(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);
		static void Destroy(Ptr& renderer);

		virtual ~RootRenderer();

	private:
		IRendererList mp_childRenderers;

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