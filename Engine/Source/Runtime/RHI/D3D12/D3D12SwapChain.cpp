#include "pch.h"
#include "D3D12SwapChain.h"

#include "D3D12CommandQueue.h"
#include "D3D12RenderTarget.h"

#include "Core/Container/Vector.h"

namespace Influx
{
	Ptr<D3D12SwapChain> D3D12SwapChain::Create(const Ptr<D3D12API> api, const SwapChainDesc& desc, Ptr<ID3D12CommandQueue> cmdQueue)
	{
		Ptr<D3D12SwapChain> swapChain = new D3D12SwapChain(desc);

		/* Create Dx12 SwapChain... */
		swapChain->DxSwapChain = D3D12API::CreateSwapChain((HWND)desc.WindowHandle, cmdQueue, desc.Width, desc.Height, StatNumBackBuffers);
		swapChain->CurrentBackBufferIndex = swapChain->DxSwapChain->GetCurrentBackBufferIndex();

		// Create (Colour)Render Targets:
		for (uint32_t i = 0; i < StatNumBackBuffers; ++i)
		{
			/* Get swapchain buffer resource */
			swapChain->DxSwapChain->GetBuffer(i, IID_PPV_ARGS(&swapChain->BackBufferResources[i]));

			swapChain->BackBufferRenderTargets[i] = D3D12RenderTarget::CreateFromResource(api, {desc.Width, desc.Height}, 
				ERHIFormat::RGBA_8_Unorm, RHIRenderTarget::ERenderTargetType::ColourTarget, swapChain->BackBufferResources[i]);
		}

		// Create (Depth)Render Target:
		RHIRenderTarget::RenderTargetConfig config{};
		config.ClearValue = { 1.0f, 0.0f, 0.0f, 0.0f };
		swapChain->BackBufferDepthTarget = D3D12RenderTarget::CreateDepthStencil(api, { desc.Width, desc.Height }, ERHIFormat::D_32_Float, config);

		return swapChain;
	}

	void D3D12SwapChain::Present(const PresentDescription& presentDesc)
	{
		UINT syncIntv = presentDesc.VSync ? 1 : 0;
		UINT flags = RHISwapChain::StatTearingSupported && !presentDesc.VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		DxSwapChain->Present(syncIntv, 0);
		CurrentBackBufferIndex = DxSwapChain->GetCurrentBackBufferIndex();
	}

	Ptr<RHIRenderTarget> D3D12SwapChain::GetCurrentRenderTarget() const 
	{
		return BackBufferRenderTargets[GetCurrentBackBufferIndex()];
	}

	Ptr<RHIRenderTarget> D3D12SwapChain::GetDepthTarget() const
	{
		return BackBufferDepthTarget;
	}

	void D3D12SwapChain::Resize(const Ptr<RenderAPI> api, Ptr<RHICommandQueue> cmdQueue, const Vector2u& newSize)
	{
		// Flush the commandqueue
		cmdQueue->Flush();

		// Resize RenderTargets:
		for (Ptr<RHIRenderTarget> rt : BackBufferRenderTargets)
		{
			rt->Resize(api, newSize);
		}
		
		BackBufferDepthTarget->Resize(api, newSize);
	}

	ID3D12Resource* D3D12SwapChain::GetCurrentBackBufferResource() const
	{
		return BackBufferResources[GetCurrentBackBufferIndex()];
	}

	Ptr<ID3D12Resource> D3D12SwapChain::GetDepthBufferResource() const
	{
		return DepthBufferResource;
	}

	D3D12SwapChain::~D3D12SwapChain()
	{

	}
}

