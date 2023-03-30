
#include "InfluxGraphics.h"

#include "Core/Platform/WindowsPlatform.h"

int main()
{
	using namespace Influx;

	Graphics::SetDebugLayerEnabled();

	const Math::Vectoru2 dimensions = { 640u, 480u };

	if (Platform::WindowHandle windowHandle = Platform::CreateWindow({ dimensions, "Window " }, true))
	{
		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Graphics::RHISwapchainDesc::EBuffering::Triple;
		swapchainDesc.Dimensions	= dimensions;
		swapchainDesc.WindowHandle	= windowHandle;

		Graphics::Create(Graphics::EGraphicsAPI::D3D12, [swapchainDesc]()
			{
				if (Graphics::RHISwapchainHandle swapchainHandle; 
					Graphics::CreateSwapchain(swapchainDesc, swapchainHandle))
				{
					// Dispatch work to GPU
					Graphics::DispatchGraphicsCommands([swapchainHandle](const Graphics::RHIGraphicsCommandListHandle& cmdList)
					{
						Graphics::Commands::ClearSwapchainBackBuffer(cmdList, swapchainHandle, { 1,0,0,1 });
					});


					// Present the swapchain back-buffer
					Graphics::DispatchSwapchainPresent(swapchainHandle);
				}
			});
	}
}