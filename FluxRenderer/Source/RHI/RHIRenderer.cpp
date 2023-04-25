#include "RHIRenderer.h"

#include "InfluxGraphics.h"
#include "Core/Platform/WindowsPlatform.h"

namespace Influx
{
	void RHIRenderer::BuildRenderWork(Platform::WindowHandle windowHandle)
	{
		
	}

	void RHIRenderer::PresentToWindow(Platform::WindowHandle windowHandle)
	{
		Graphics::RHISwapchainDesc desc{};
		desc.Buffering = Graphics::RHISwapchainDesc::EBuffering::Triple;
		desc.Dimensions = Platform::GetClientWindowDimensions<uint32>(windowHandle);

		if (m_swapchain.IsValid() || Graphics::CreateSwapchain(desc, m_swapchain))
		{
			Graphics::DispatchGraphicsCommands([this](const Graphics::RHICommandListHandle& commandList)
			{
				Graphics::Commands::ClearSwapchainBackBuffer(commandList, m_swapchain, { 1, 0, 0, 1 });
			});
		}
	}

	void RHIRenderer::Initialize()
	{
		if (Graphics::GetInitializedGraphicsAPI() == Graphics::EGraphicsAPI::Max)
		{
			Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);
		}
	}

	void RHIRenderer::Cleanup()
	{
		Graphics::Cleanup();
	}
}


