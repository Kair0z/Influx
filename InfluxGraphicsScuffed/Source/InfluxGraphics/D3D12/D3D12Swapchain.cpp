#include "InfluxGraphics/D3D12/D3D12Swapchain.h"

namespace Influx::Graphics
{
	D3D12Swapchain::D3D12Swapchain(uint32 width, uint32 height, bool isTearingSupported)
		: RHISwapchain(width, height, isTearingSupported)
	{

	}

	void D3D12Swapchain::Present(RHICommandQueue* commandQueue, bool VSync)
	{

	}

	void D3D12Swapchain::Resize(RHIDevice* device, RHICommandQueue* commandQueue, const Math::Vectoru2& newDimensions)
	{

	}

	ID3D12DescriptorHeap* D3D12Swapchain::GetDxRtvDescriptorHeap() const
	{
		return mp_dxRenderTargetDescriptorHeap;
	}

	D3D12Swapchain::~D3D12Swapchain()
	{
		D3D12::SafeRelease(mp_dxgiSwapchain);
	}
}