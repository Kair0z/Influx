#include "RHIRenderer.h"

#include "InfluxGraphics.h"
#include "Core/Platform/WindowsPlatform.h"

namespace influx
{
	void RHIRenderer::RecordRenderCommands(platform::window_handle windowHandle)
	{
		
	}

	void RHIRenderer::PresentToWindow(platform::window_handle windowHandle)
	{
		Graphics::RHISwapchainDesc desc{};
		desc.Buffering = Graphics::RHISwapchainDesc::EBuffering::Triple;
		desc.Dimensions = platform::GetClientWindowDimensions<uint32>(windowHandle);

		if (m_swapchain.IsValid() || Graphics::CreateSwapchain(desc, m_swapchain))
		{
			// Graphics::DispatchGraphicsCommands([this](const Graphics::RHICommandListHandle& commandList)
			// {
			// 	Graphics::Commands::ClearSwapchainBackBuffer(commandList, m_swapchain, { 1, 0, 0, 1 });
			// });

			Graphics::DispatchSwapchainPresent(m_swapchain, {});
		}
	}

	RHIRenderer::RHIRenderer()
	{
		if (Graphics::GetInitializedGraphicsAPI() == Graphics::EGraphicsAPI::Max)
		{
			Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);
		}
	}

	RHIRenderer::~RHIRenderer()
	{
		Graphics::Cleanup();
	}
}


