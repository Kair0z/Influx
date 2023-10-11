
#include "InfluxGraphics.h"

#include "Core/BasicTypes.h"
#include "Core/Platform/WindowsPlatform.h"
#include "Core/Registry.h"

namespace influx
{
	struct Settings final
	{
		constexpr static uint32 NumFrames = 4096u * 4096u;
		constexpr static uint32 WindowWidth = 640u;
		constexpr static uint32 WindowHeight = 480u;

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
	using namespace influx;

#ifdef _DEBUG
	Graphics::SetDebugLayerEnabled();
#endif

	const Math::Vectoru2 dimensions { Settings::WindowWidth, Settings::WindowHeight };

	// Create Platform window...
	if (platform::window_handle windowHandle = platform::CreateWindow({ dimensions, Settings::WindowName }))
	{
		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Settings::SwapchainBuffering;
		swapchainDesc.Dimensions	= dimensions;
		swapchainDesc.WindowHandle	= windowHandle;

		Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);

		if (Graphics::RHISwapchainHandle swapchain; Graphics::CreateSwapchain(swapchainDesc, swapchain))
		{
			for (uint64 i = 0u; i < Settings::NumFrames; ++i)
			{
				Graphics::DispatchGraphicsCommands([&swapchain](const Graphics::RHICommandListHandle& cmdList)
				{
					Graphics::Commands::ClearSwapchainBackBuffer(cmdList, swapchain, {1, 0, 0, 1});
				});
			}
		}

		Graphics::Cleanup();
	}
}