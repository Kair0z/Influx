#pragma once

#ifndef __RENDERER_ROOT_RENDERER_H_
#define __RENDERER_ROOT_RENDERER_H_

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
	class RootRenderer final
	{
		using Ptr = RootRenderer*;

	public:
		typedef void (*OnBuildCommandList)(Graphics::RHICommandList* cmdList);
		typedef void (*OnWindowResize)(const Math::Vectoru2& newSize);
		
		RootRenderer(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);
		static Ptr Create(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle = nullptr);
		static void Destroy(Ptr& renderer);
		virtual ~RootRenderer();

		void Render();
		void Render(OnBuildCommandList internalRenderClb);
		void Present(bool vsync);

		bool AttachToWindow(Platform::WindowHandle windowHandle);
		bool DetachFromCurrentWindow();
		bool IsAttachedToWindow() const;

		bool SignalWindowResize(const Math::Vectoru2& newSize);
		bool DoesSwapchainNeedResize() const;

		void SetGraphicsAPI(const Graphics::EGraphicsAPI api);
		Graphics::EGraphicsAPI GetCurrentGraphicsAPI() const;

		static bool IsGraphicsAPISupported(const Graphics::EGraphicsAPI api);

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