#pragma once

#ifndef __RENDERER_RENDERER_H_
#define __RENDERER_RENDERER_H_

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"
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

	public:
		/* Submitting work onto a passed RHICommandList */
		virtual void OnRender(RHICommandListPtr commandList) const = 0;

		/* Initializing RHI Resources */
		virtual void Initialize(const RHIDevicePtr) = 0;

		/* Cleaning up RHI Resources */
		virtual void Cleanup(const RHIDevicePtr) = 0;

	protected:
		IRenderer();

	public:
		IRenderer(const IRenderer&) = delete;
		IRenderer(IRenderer&&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;
		IRenderer& operator=(IRenderer&&) = delete;
		virtual ~IRenderer();
	};

	class RootRenderer final
	{
	public:
		typedef void (*OnRenderClb)(Graphics::RHICommandList* cmdList);

		void Render();
		void Render(OnRenderClb internalRenderClb);
		void Present(bool vsync);

		void Initialize(const Graphics::EGraphicsAPI api);

		bool AttachToWindow(Platform::WindowHandle windowHandle);
		bool IsAttachedToWindow() const;

		void SetGraphicsAPI(const Graphics::EGraphicsAPI api);
		Graphics::EGraphicsAPI GetGraphicsAPI() const;

		uint64 GetFrame() const;

		Graphics::RHIDevice* GetDevice() const;

	public:
		RootRenderer();
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

		Graphics::RHISwapchain* mp_windowSwapchain;

		uint64 m_frame;

		void StartRender(Graphics::RHICommandList* cmdList) const;
		void FinishRender(Graphics::RHICommandList* cmdList) const;

		void ClearWindowSwapchain(Graphics::RHICommandList* cmdList, const Math::Vectorf4& clearColour) const;

		void InitializeDevice(const Graphics::EGraphicsAPI api);
		void Cleanup();
		bool IsInitialized(const Graphics::EGraphicsAPI api) const;
	};
}

#endif
