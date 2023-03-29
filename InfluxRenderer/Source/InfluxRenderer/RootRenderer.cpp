#include "renderer_pch.h"
#include "InfluxRenderer/RootRenderer.h"

#include "InfluxGraphics/RHI.h"
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
			mp_rhiDevice = new D3D12Device(true);
			m_initializedDeviceAPI = api;
			break;

		case EGraphicsAPI::NotSupported:
		default:
			m_initializedDeviceAPI = EGraphicsAPI::NotSupported;
			Cleanup();
			return;
		}

		// Run Child Renderers:
		for (IRenderer* renderer : mp_childRenderers)
		{
			renderer->OnPostInitializeAPI(m_initializedDeviceAPI, GetDevice());
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
		// Run Child Renderers:
		for (IRenderer* renderer : mp_childRenderers)
		{
			renderer->OnPreCleanupAPI(GetCurrentGraphicsAPI(), GetDevice());
		}

		CleanupGraphicsAPI(GetCurrentGraphicsAPI());
	}

	void RootRenderer::UpdateSwapchain()
	{
		const bool isAttachedToWindow = IsAttachedToWindow();
		if (!isAttachedToWindow)
		{
			return;
		}

		// Run Child Renderers:
		for (IRenderer* renderer : mp_childRenderers)
		{
			renderer->OnWindowResize(GetRenderContext(), 
				mp_windowSwapchain->m_previousSize, 
				mp_windowSwapchain->m_updatedSize);
		}

		mp_windowSwapchain->m_isDirty = false;
		mp_windowSwapchain->m_previousSize = mp_windowSwapchain->m_updatedSize;
		mp_windowSwapchain->m_updatedSize = mp_windowSwapchain->m_previousSize;
	}

	const RenderContext& RootRenderer::GetRenderContext()
	{
		m_renderContext.mp_rootRendererPtr = this;
		return m_renderContext;
	}

	void RootRenderer::Render()
	{
		Render(nullptr);
	}

	void RootRenderer::Render(OnBuildCommandList internalRenderClb)
	{
		if (IsAttachedToWindow())
		{
			UpdateSwapchain();

			RHICommandList* cmdList = mp_gfxCommandQueue->SetupNewCommandList(GetDevice());

			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::RenderTarget);
			{
				cmdList->ClearRTV(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentRenderTargetView(), { 0.2f, 0.0f, 0.2f, 1.0f });

				if (internalRenderClb != nullptr)
				{
					// Run an internal Render Clb passed...
					internalRenderClb(cmdList);
				}
				else
				{
					// Run Child Renderers...
					for (IRenderer* renderer : mp_childRenderers)
					{
						renderer->OnBuildRenderCommandList(GetRenderContext(), cmdList);
					}
				}
			}
			cmdList->TransitionResource(mp_windowSwapchain->mp_rhiSwapchain->GetCurrentBackBufferResource(), Graphics::ERHIResourceState::Present);

			mp_gfxCommandQueue->ExecuteCommmandList(cmdList);

			++m_frame;
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

	RootRenderer::GraphicsPipelinePtr RootRenderer::GetAndOrCreateGraphicsPipeline(const GfxPipelineKey& key, const GfxPipelineLayoutKey& pipelineLayoutKey)
	{
		GraphicsPipelineLayoutPtr pipelineLayout = GetAndOrCreateGraphicsPipelineLayout(pipelineLayoutKey);

		if (!mp_graphicsPipelineCache.Contains(key))
		{
			// Create new RHI Graphics Pipeline
			GraphicsPipelinePtr newPipeline = mp_rhiDevice->CreateGraphicsPipeline(key, pipelineLayout);
			mp_graphicsPipelineCache.Add(key, newPipeline);
		}
		
		return *mp_graphicsPipelineCache.Get(key);
	}

	RootRenderer::GraphicsPipelineLayoutPtr RootRenderer::GetAndOrCreateGraphicsPipelineLayout(const GfxPipelineLayoutKey& key)
	{
		if (!mp_graphicsPipelineLayoutCache.Contains(key))
		{
			// Create new RHI Graphics Pipeline Layout
			GraphicsPipelineLayoutPtr newPipelineLayout = mp_rhiDevice->CreateGraphicsPipelineLayout();
			mp_graphicsPipelineLayoutCache.Add(key, newPipelineLayout);
		}
		
		return *mp_graphicsPipelineLayoutCache.Get(key);
	}

	RootRenderer::TexturePtr RootRenderer::GetAndOrCreateTexture(const String& key, const Graphics::RHITextureDesc& desc)
	{
		if (!mp_textureCache.Contains(key))
		{
			// Create new RHI Texture
			TexturePtr newTexture = mp_rhiDevice->CreateTexture(Graphics::ERHIResourceState::GenericRead, desc);
			mp_textureCache.Add(key, newTexture);
		}

		return *mp_textureCache.Get(key);
	}

	RootRenderer::SwapchainPtr RootRenderer::GetWindowSwapchain() const
	{
		return mp_windowSwapchain->mp_rhiSwapchain;
	}

	const Math::Vectoru2& RootRenderer::GetWindowSwapchainDimensions() const
	{
		return (mp_windowSwapchain->m_isDirty) ? mp_windowSwapchain->m_previousSize : mp_windowSwapchain->m_updatedSize;
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

#pragma region RenderContext
	const Graphics::RHIDevice* RenderContext::GetDevice() const
	{
		return mp_rootRendererPtr->GetDevice();
	}

	Graphics::RHITexture* RenderContext::GetAndOrCreateTexture(const String& key, const Graphics::RHITextureDesc& desc) const
	{
		return mp_rootRendererPtr->GetAndOrCreateTexture(key, desc);
	}

	Graphics::RHIGraphicsPipeline* RenderContext::GetAndOrCreateGraphicsPipeline(const Graphics::RHIGraphicsPipelineDescription& key, const Graphics::RHIGraphicsPipelineLayoutDescription& pipelineLayoutKey) const
	{
		return mp_rootRendererPtr->GetAndOrCreateGraphicsPipeline(key, pipelineLayoutKey);
	}

	Graphics::RHIGraphicsPipelineLayout* RenderContext::GetAndOrCreateGraphicsPipelineLayout(const Graphics::RHIGraphicsPipelineLayoutDescription& key) const
	{
		return mp_rootRendererPtr->GetAndOrCreateGraphicsPipelineLayout(key);
	}

	bool RenderContext::CopyTextureIntoSwapchain(Graphics::RHITexture* texture, Graphics::RHICommandList* cmdList) const
	{
		cmdList->CopyResource(texture->GetResource(), GetSwapchain()->GetCurrentBackBufferResource());
		return true;
	}

	Graphics::RHISwapchain* RenderContext::GetSwapchain() const
	{
		return mp_rootRendererPtr->GetWindowSwapchain();
	}
#pragma endregion
}