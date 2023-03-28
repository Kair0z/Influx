
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
			struct FrameBuffer final
			{
				uint64 Index;
				Graphics::RHIGraphicsCommandBufferHandle CommandBuffer;

				bool IsInFlight()
				{
					return (CommandBuffer != nullptr) && (CommandBuffer);
				}
			}
			FrameBuffers[static_cast<uint8>(Settings::Buffering)];

			uint64 CurrentFrame = 0u;

			if (Graphics::RHIGraphicsCommandQueueHandle graphicsQueue; Graphics::CreateGraphicsCommandQueue(graphicsQueue))
			{
				while (true)
				{
					FrameBuffer& fb = FrameBuffers[CurrentFrame % 3u];

					while (fb.IsInFlight()) { /* Wait... */ }

					fb.Index = CurrentFrame;
					
					// Get a commandBuffer
					if (Graphics::RHIGraphicsCommandBufferHandle commandBuffer; Graphics::CreateGraphicsCommandBuffer(fb.CommandBuffer))
					{
						
					}

					++CurrentFrame;
				}
			}
			
		});
}