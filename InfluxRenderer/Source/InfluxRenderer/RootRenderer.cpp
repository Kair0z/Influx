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

	RootRenderer::RootRenderer(const EGraphicsAPI api, Platform::WindowHandle windowHandle)
	{
		Initialize(api);
		AttachToWindow(windowHandle);
	}

	RootRenderer::Ptr RootRenderer::Create(const Graphics::EGraphicsAPI api, Platform::WindowHandle windowHandle)
	{
		return new RootRenderer(api, windowHandle);
	}

	RootRenderer::~RootRenderer()
	{
		Cleanup();
	}

	void RootRenderer::Destroy(Ptr& renderer)
	{
		if (renderer != nullptr)
		{
			delete renderer;
			renderer = nullptr;
		}
	}


	void RootRenderer::Initialize(const Graphics::EGraphicsAPI api)
	{
		if (api == Graphics::EGraphicsAPI::NotSupported)
		{
			return;
		}

		if (IsGraphicsInitialized(api))
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
		if (!IsGraphicsInitialized(GetGraphicsAPI()))
		{
			Initialize(GetGraphicsAPI());
			return;
		}

		InitializeChildRenderers();

		if (IsAttachedToWindow())
		{
			AttachChildRenderersToSwapchain();
		}

		UpdateSwapchain();

		/* Create a commandlist */
		RHICommandList* cmdList = mp_gfxCommandQueue->SetupNewCommandList(GetDevice());

		StartRender(cmdList);

		{
			/* First, render the child-render-list */
			for (const IRenderer* renderer : GetChildRendererList())
			{
				if (renderer->IsInitialized())
				{
					renderer->Render(cmdList);
				}
			}

			/* Then, command the optional extra lambda passed... */
			if (internalRenderClb != nullptr)
			{
				internalRenderClb(cmdList);
			}
		}

		FinishRender(cmdList);

		mp_gfxCommandQueue->ExecuteCommmandList(cmdList);

		++m_frame;
	}

	void RootRenderer::StartRender(Graphics::RHICommandList* cmdList) const
	{
		if (IsAttachedToWindow())
		{
			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::RenderTarget);

			// [ TEMP ]
			cmdList->ClearRTV(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentRenderTargetView(), { 0.2f, 0.0f, 0.2f, 1.0f });
		}
	}

	void RootRenderer::FinishRender(Graphics::RHICommandList* cmdList) const
	{
		if (IsAttachedToWindow())
		{
			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::Present);
		}
	}

	void RootRenderer::Present(bool vsync)
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		mp_windowSwapchain->mp_rhiSwapchain->Present(mp_gfxCommandQueue, vsync);
	}

	void RootRenderer::Cleanup()
	{
		if (!IsGraphicsInitialized(GetGraphicsAPI()))
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

	void RootRenderer::InitializeChildRenderers()
	{
		if (!IsGraphicsInitialized(GetGraphicsAPI()))
		{
			return;
		}

		for (IRenderer* renderer : GetChildRendererList())
		{
			if (renderer->IsInitialized())
			{
				continue;
			}

			renderer->Initialize(GetDevice());
		}
	}

	void RootRenderer::AttachChildRenderersToSwapchain()
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		for (IRenderer* renderer : GetChildRendererList())
		{
			renderer->AttachToRenderTarget(nullptr, nullptr);
		}
	}

	void RootRenderer::DetachChildRenderersToSwapchain()
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		for (IRenderer* renderer : GetChildRendererList())
		{
			if (renderer->IsAttachedToRenderTarget())
			{
				renderer->DetachFromRenderTarget(nullptr);
			}
		}
	}

	void RootRenderer::CleanupChildRenderers(bool)
	{
		if (!IsGraphicsInitialized(GetGraphicsAPI()))
		{
			// >:(
			return;
		}

		DetachFromCurrentWindow();

		for (IRenderer* renderer : GetChildRendererList())
		{
			if (!renderer->IsInitialized())
			{
				continue;
			}

			renderer->Cleanup(GetDevice());
		}
	}

	void RootRenderer::UpdateSwapchain()
	{
		if (!IsAttachedToWindow())
		{
			return;
		}

		if (!mp_windowSwapchain->m_isDirty)
		{
			return;
		}

		for (IRenderer* renderer : GetChildRendererList())
		{
			if (renderer->IsAttachedToRenderTarget())
			{
				renderer->ResizeRenderTarget(nullptr);
			}
		}
	}

	bool RootRenderer::AttachToWindow(Platform::WindowHandle windowHandle)
	{
		if (!IsGraphicsInitialized(GetGraphicsAPI()))
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

		mp_windowSwapchain = new RootRenderer::SwapchainTarget();
		mp_windowSwapchain->mp_rhiSwapchain = GetDevice()->CreateSwapchain(windowRect.m_widthHeigth, windowHandle, mp_gfxCommandQueue);

		for (IRenderer* renderer : GetChildRendererList())
		{
			renderer->AttachToRenderTarget(nullptr, nullptr);
		}

		OnWindowResize(windowRect.m_widthHeigth);

		return IsAttachedToWindow();
	}

	bool RootRenderer::DetachFromCurrentWindow()
	{
		if (!IsAttachedToWindow())
		{
			return true;
		}

		DetachChildRenderersToSwapchain();

		// ToDelete...
		//mp_windowSwapchain->mp_rhiSwapchain;

		delete mp_windowSwapchain;
		mp_windowSwapchain = nullptr;
		return true;
	}

	bool RootRenderer::IsAttachedToWindow() const
	{
		return mp_windowSwapchain != nullptr && mp_windowSwapchain->mp_rhiSwapchain != nullptr;
	}

	bool RootRenderer::OnWindowResize(const Math::Vectoru2& newSize)
	{
		if (!IsAttachedToWindow())
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

	Graphics::EGraphicsAPI RootRenderer::GetGraphicsAPI() const
	{
		return m_currentGraphicsAPI;
	}

	uint64 RootRenderer::GetFrame() const
	{
		return m_frame;
	}

	bool RootRenderer::IsGraphicsInitialized(const Graphics::EGraphicsAPI api) const
	{
		return (m_initializedDeviceAPI == api) && (m_initializedDeviceAPI != Graphics::EGraphicsAPI::NotSupported);
	}

	Graphics::RHIDevice* RootRenderer::GetDevice() const
	{
		return mp_rhiDevice;
	}

	const RootRenderer::IRendererList& RootRenderer::GetChildRendererList() const
	{
		return mp_childRenderers;
	}

	RootRenderer::IRendererList& RootRenderer::GetChildRendererList()
	{
		return mp_childRenderers;
	}
}