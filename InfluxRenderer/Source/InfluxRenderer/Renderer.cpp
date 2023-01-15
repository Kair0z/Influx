#include "renderer_pch.h"
#include "InfluxRenderer/Renderer.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "Core/Platform/WindowsPlatform.h"

namespace Influx::Renderer
{
	using namespace Influx::Graphics;

	RootRenderer::RootRenderer()
	{
		
	}

	RootRenderer::RootRenderer(const EGraphicsAPI api, Platform::WindowHandle windowHandle)
	{
		Initialize(api);
		AttachToWindow(windowHandle);
	}

	RootRenderer::~RootRenderer()
	{
		Cleanup();
	}


	void RootRenderer::Initialize(const Graphics::EGraphicsAPI api)
	{
		if (api == Graphics::EGraphicsAPI::NotSupported)
		{
			return;
		}

		if (IsInitialized(api))
		{
			return;
		}

		InitializeDevice(api);

		mp_gfxCommandQueue = GetDevice()->CreateCommandQueue(ERHICommandQueueType::Graphics);
	}

	void RootRenderer::InitializeDevice(const Graphics::EGraphicsAPI api)
	{
		m_initializedDeviceAPI = api;

		switch (api)
		{
		case EGraphicsAPI::D3D12:
			mp_rhiDevice = new D3D12Device();
			break;

		case EGraphicsAPI::NotSupported:
		default:
			Cleanup();
			break;
		}

		SetGraphicsAPI(api);
	}

	void RootRenderer::Render()
	{
		Render(nullptr);
	}

	void RootRenderer::Render(OnRenderClb internalRenderClb)
	{
		if (!IsInitialized(GetGraphicsAPI()))
		{
			Initialize(GetGraphicsAPI());
			return;
		}

		RHICommandList* cmdList = mp_gfxCommandQueue->SetupNewCommandList(GetDevice());

		StartRender(cmdList);

		if (internalRenderClb != nullptr)
		{
			internalRenderClb(cmdList);
		}

		FinishRender(cmdList);

		mp_gfxCommandQueue->ExecuteCommmandList(cmdList);

		++m_frame;
	}

	void RootRenderer::StartRender(Graphics::RHICommandList* cmdList) const
	{
		if (IsAttachedToWindow())
		{
			cmdList->TransitionResource(mp_windowSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::RenderTarget);

			// [ TEMP ]
			cmdList->ClearRTV(mp_windowSwapchain->GetCurrentRenderTargetView(), { 0.2f, 0.0f, 0.2f, 1.0f });
		}
	}

	void RootRenderer::FinishRender(Graphics::RHICommandList* cmdList) const
	{
		if (IsAttachedToWindow())
		{
			cmdList->TransitionResource(mp_windowSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::Present);
		}
	}

	void RootRenderer::Present(bool vsync)
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		mp_windowSwapchain->Present(mp_gfxCommandQueue, vsync);
	}

	void RootRenderer::Cleanup()
	{
		if (!IsInitialized(GetGraphicsAPI()))
		{
			return;
		}

		mp_gfxCommandQueue->Flush();
	}

	bool RootRenderer::AttachToWindow(Platform::WindowHandle windowHandle)
	{
		if (!IsInitialized(GetGraphicsAPI()))
		{
			Initialize(GetGraphicsAPI());
		}

		if (windowHandle == nullptr)
		{
			return false;
		}

		if (IsAttachedToWindow())
		{
			// Todo... Reattach?
			return false;
		}

		Math::Rectu windowRect = Platform::GetClientWindowRect<uint32>(windowHandle);

		mp_windowSwapchain = GetDevice()->CreateSwapchain(windowRect.m_widthHeigth, windowHandle, mp_gfxCommandQueue);

		return IsAttachedToWindow();
	}

	bool RootRenderer::IsAttachedToWindow() const
	{
		return mp_windowSwapchain != nullptr;
	}

	void RootRenderer::SetGraphicsAPI(const Graphics::EGraphicsAPI api)
	{
		m_currentGraphicsAPI = api;
	}

	Graphics::EGraphicsAPI RootRenderer::GetGraphicsAPI() const
	{
		return m_currentGraphicsAPI;
	}

	uint64 RootRenderer::GetFrame() const
	{
		return m_frame;
	}

	bool RootRenderer::IsInitialized(const Graphics::EGraphicsAPI api) const
	{
		return (m_initializedDeviceAPI == api) && (m_initializedDeviceAPI != Graphics::EGraphicsAPI::NotSupported);
	}

	Graphics::RHIDevice* RootRenderer::GetDevice() const
	{
		return mp_rhiDevice;
	}
}