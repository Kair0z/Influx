#include "InfluxGraphics/RHISwapchain.h"

namespace Influx::Graphics
{
	RHISwapchain::RHISwapchain(uint32 width, uint32 height, bool isTearingSupported)
		: m_width{ width }
		, m_height{ height }
		, m_isTearingSupported{ isTearingSupported }
	{

	}

	RHIResource* RHISwapchain::GetCurrentBackBufferResource()
	{
		return mp_backBufferResources[GetCurrentBackBufferIndex()];
	}

	RHIRenderTargetView* RHISwapchain::GetCurrentRenderTargetView()
	{
		return mp_backBufferRTVs[GetCurrentBackBufferIndex()];
	}
}