#include "renderer_pch.h"
#include "InfluxRenderer/RootRenderer.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "Core/Platform/WindowsPlatform.h"

namespace Influx::Renderer
{
	using namespace Influx::Graphics;

	RootRenderer::RootRenderer(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle)
	{
		InitializeGraphicsAPI(api);
		AttachToWindow(windowHandle);
	}
	
	RootRenderer::~RootRenderer()
	{
		Cleanup();
	}

	RootRenderer::Ptr RootRenderer::Create(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle)
	{
		return new RootRenderer(api, windowHandle);
	}

	void RootRenderer::Destroy(Ptr& renderer)
	{
		if (renderer != nullptr)
		{
			delete renderer;
			renderer = nullptr;
		}
	}

	void RootRenderer::InitializeGraphicsAPI(const Graphics::EGraphicsAPI api)
	{
		const bool isAPISupported = IsGraphicsAPISupported(api);
		const bool isAPIAlreadyInitialized = IsGraphicsAPIInitialized(api);

		if (!isAPISupported || isAPIAlreadyInitialized)
		{
			return;
		}

		// Create Graphics API Device:
		SetGraphicsAPI(api);
		switch (api)
		{
		case EGraphicsAPI::D3D12:
			mp_rhiDevice = new D3D12Device();
			break;

		case EGraphicsAPI::NotSupported:
		default:
			Cleanup();
			return;
		}

		// Initialize Graphics Command Queue:
		mp_gfxCommandQueue = GetDevice()->CreateCommandQueue(ERHICommandQueueType::Graphics);
	}

	void RootRenderer::CleanupGraphicsAPI(const Graphics::EGraphicsAPI api)
	{
		const bool isCurrentAPIInitialized		= IsGraphicsAPIInitialized(api);

		if (!isCurrentAPIInitialized)
		{
			return;
		}

		if (IsAttachedToWindow())
		{
			DetachFromCurrentWindow();
		}

		if (mp_windowSwapchain)
		{
			delete mp_windowSwapchain;
			mp_windowSwapchain = nullptr;
		}

		mp_gfxCommandQueue->Flush();
	}

	void RootRenderer::Cleanup()
	{
		CleanupGraphicsAPI(GetCurrentGraphicsAPI());
	}

	void RootRenderer::UpdateSwapchain()
	{

	}

	void RootRenderer::Render()
	{
		Render(nullptr);
	}

	void RootRenderer::Render(OnBuildCommandList internalRenderClb)
	{
		UpdateSwapchain();

		if (IsAttachedToWindow())
		{
			RHICommandList* cmdList = mp_gfxCommandQueue->SetupNewCommandList(GetDevice());

			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::RenderTarget);

			cmdList->ClearRTV(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentRenderTargetView(), { 0.2f, 0.0f, 0.2f, 1.0f });

			if (internalRenderClb != nullptr)
			{
				internalRenderClb(cmdList);
			}

			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::Present);

			mp_gfxCommandQueue->ExecuteCommmandList(cmdList);
		}

		++m_frame;
	}

	void RootRenderer::Present(bool vsync)
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		mp_windowSwapchain->mp_rhiSwapchain->Present(mp_gfxCommandQueue, vsync);
	}

	bool RootRenderer::AttachToWindow(Platform::WindowHandle windowHandle)
	{
		const bool isAPIInitialized = IsGraphicsAPIInitialized();

		if (!isAPIInitialized)
		{
			return false;
		}

		if (windowHandle == nullptr)
		{
			return false;
		}

		const bool isAlreadyAttachedToWindow = IsAttachedToWindow();
		if (isAlreadyAttachedToWindow)
		{
			// Todo... Reattach?
			return false;
		}

		Math::Rectu windowRect = Platform::GetClientWindowRect<uint32>(windowHandle);

		mp_windowSwapchain = new RootRenderer::SwapchainTarget();
		mp_windowSwapchain->mp_rhiSwapchain = GetDevice()->CreateSwapchain(windowRect.m_widthHeigth, windowHandle, mp_gfxCommandQueue);

		SignalWindowResize(windowRect.m_widthHeigth);

		return IsAttachedToWindow();
	}

	bool RootRenderer::DetachFromCurrentWindow()
	{
		if (!IsAttachedToWindow())
		{
			return true;
		}

		if (mp_windowSwapchain)
		{
			delete mp_windowSwapchain;
			mp_windowSwapchain = nullptr;
		}
		
		return true;
	}

	bool RootRenderer::IsAttachedToWindow() const
	{
		return mp_windowSwapchain != nullptr && mp_windowSwapchain->mp_rhiSwapchain != nullptr;
	}

	bool RootRenderer::SignalWindowResize(const Math::Vectoru2& newSize)
	{
		const bool isAttachedToWindow = IsAttachedToWindow();
		if (!isAttachedToWindow)
		{
			return false;
		}

		mp_windowSwapchain->m_isDirty = true;
		mp_windowSwapchain->m_previousSize = mp_windowSwapchain->m_updatedSize;
		mp_windowSwapchain->m_updatedSize = newSize;

		return true;
	}

	bool RootRenderer::DoesSwapchainNeedResize() const
	{
		if (!IsAttachedToWindow())
		{
			return false;
		}

		return mp_windowSwapchain->m_isDirty;
	}

	void RootRenderer::SetGraphicsAPI(const Graphics::EGraphicsAPI api)
	{
		m_currentGraphicsAPI = api;
	}

	Graphics::EGraphicsAPI RootRenderer::GetCurrentGraphicsAPI() const
	{
		return m_currentGraphicsAPI;
	}

	bool RootRenderer::IsGraphicsAPISupported(const Graphics::EGraphicsAPI api)
	{
		return (api < EGraphicsAPI::NotSupported);
	}

	uint64 RootRenderer::GetFrame() const
	{
		return m_frame;
	}

	bool RootRenderer::IsGraphicsAPIInitialized(const Graphics::EGraphicsAPI api) const
	{
		const bool isAPISupported = IsGraphicsAPISupported(api);

		if (!isAPISupported)
		{
			return false;
		}

		return (m_initializedDeviceAPI == api);
	}

	bool RootRenderer::IsGraphicsAPIInitialized() const
	{
		return IsGraphicsAPIInitialized(GetCurrentGraphicsAPI());
	}

	Graphics::RHIDevice* RootRenderer::GetDevice() const
	{
		return mp_rhiDevice;
	}
}