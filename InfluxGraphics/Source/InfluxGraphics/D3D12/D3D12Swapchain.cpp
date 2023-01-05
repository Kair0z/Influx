#include "InfluxGraphics/D3D12/D3D12Swapchain.h"
#include "InfluxGraphics/RHICommandQueue.h"

namespace Influx::Graphics
{
	D3D12Swapchain::D3D12Swapchain(uint32 width, uint32 height, bool isTearingSupported)
		: RHISwapchain(width, height, isTearingSupported)
	{

	}

	void D3D12Swapchain::Present(RHICommandQueue* commandQueue, bool VSync)
	{
		UINT syncIntv = VSync ? 1 : 0;
		UINT flags = GetIsTearingSupported() && !VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		GetDxgiSwapchain()->Present(syncIntv, 0);

		m_currentBackBufferIndex = GetDxgiSwapchain()->GetCurrentBackBufferIndex();
	}

	void D3D12Swapchain::Resize(RHIDevice* device, RHICommandQueue* commandQueue, const Math::Vectoru2& newDimensions)
	{
		commandQueue->Flush();
	}

	ID3D12DescriptorHeap* D3D12Swapchain::GetDxRtvDescriptorHeap() const
	{
		return mp_dxRenderTargetDescriptorHeap;
	}

	IDXGISwapChain4* D3D12Swapchain::GetDxgiSwapchain() const
	{
		return mp_dxgiSwapchain;
	}

	D3D12Swapchain::~D3D12Swapchain()
	{
		D3D12::SafeRelease(mp_dxgiSwapchain);
	}
}