
#include "InfluxGraphics.h"

#include "Core/BasicTypes.h"
#include "Core/Platform/WindowsPlatform.h"

#include "Core/Registry.h"

namespace Influx
{
	struct Settings final
	{
		constexpr static uint32 NumFrames = 4096u * 4096u;
		constexpr static uint32 WindowWidth = 1920u;
		constexpr static uint32 WindowHeight = 1080u;

		constexpr static const char* WindowName = "Window";

		constexpr static Graphics::RHISwapchainDesc::EBuffering SwapchainBuffering = Graphics::RHISwapchainDesc::EBuffering::Triple;

#ifdef _DEBUG
		constexpr static bool Debug = true;
#else
		constexpr static bool Debug = false;
#endif
	};
}

int main()
{
	using namespace Influx;

	Graphics::SetDebugLayerEnabled();

	const Math::Vectoru2 dimensions { Settings::WindowWidth, Settings::WindowHeight };

	// Create Platform window...
	if (Platform::WindowHandle windowHandle = Platform::CreateWindow({ dimensions, Settings::WindowName }))
	{
		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Settings::SwapchainBuffering;
		swapchainDesc.Dimensions	= dimensions;
		swapchainDesc.WindowHandle	= windowHandle;

		// Create Influx Graphics...
		Graphics::Create(Graphics::EGraphicsAPI::D3D12, [swapchainDesc]()
		{
			// Create Swapchain attachment to window...
			if (Graphics::RHISwapchainHandle swapchainHandle; Graphics::CreateSwapchain(swapchainDesc, swapchainHandle))
			{
				for (uint32 f = 0u; f < Settings::NumFrames; ++f)
				{
					// Dispatch work to GPU...
					Graphics::DispatchGraphicsCommands([swapchainHandle](const Graphics::RHIGraphicsCommandListHandle& cmdList)
					{
						Graphics::Commands::ClearSwapchainBackBuffer(cmdList, swapchainHandle, { 1,0,0,1 });
					});

					// Present the swapchain back-buffer
					constexpr static bool Vsync = true;
					Graphics::DispatchSwapchainPresent(swapchainHandle, { Vsync });
				}
			}
		});
	}
}