
#include "InfluxGraphics.h"

#include "Core/Threading/ThreadPool.h"

namespace Settings
{
	using namespace Influx;

	enum class EFrameBuffering
	{
		Single = 1,
		Double = 2,
		Triple = 3,
		Max
	};
	
	const static EFrameBuffering Buffering = EFrameBuffering::Triple;
}

int main()
{
	using namespace Influx;

	Graphics::Create(Graphics::EGraphicsAPI::D3D12, []()
	{
		uint64 CurrentFrame = 0u;

		while (true)
		{
			// Dispatch work to GPU
			Graphics::DispatchGraphicsCommands([]()
			{
				Graphics::GraphicsCmd_ClearRenderTargetView();
				
			});

			// Dispatch Swapchain to present
			Graphics::DispatchSwapchainPresent();

			++CurrentFrame;
		}
		
	});
}