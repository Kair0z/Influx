#include "pch.h"
#include "D3D12SwapChain.h"

#include "D3D12CommandQueue.h"
#include "D3D12RenderTarget.h"

#include "Core/Container/Vector.h"

namespace Influx
{
	size_t D3D12SwapChain::StatRTVDescriptorOffsetSize = 0;

	Ptr<D3D12SwapChain> D3D12SwapChain::Create(const Ptr<D3D12API> api, void* windowHandle, Ptr<ID3D12CommandQueue> cmdQueue)
	{
		Ptr<D3D12SwapChain> swapChain = new D3D12SwapChain(windowHandle);

		/* Create Dx12 SwapChain... */
		swapChain->DxSwapChain = D3D12API::CreateSwapChain((HWND)windowHandle, cmdQueue, swapChain->Width, swapChain->Height, StatNumBackBuffers);
		swapChain->CurrentBackBufferIndex = swapChain->DxSwapChain->GetCurrentBackBufferIndex();

		/* Gather Backbuffers & RendertargetViews */
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = StatNumBackBuffers;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		api->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapChain->DxRenderTargetDescriptorHeap));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapChain->DxRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		StatRTVDescriptorOffsetSize = api->GetDescriptorHandleIncrementSize_RTV();
		for (int i = 0; i < StatNumBackBuffers; ++i)
		{
			swapChain->DxSwapChain->GetBuffer(i, IID_PPV_ARGS(&swapChain->DxBackBufferResources[i]));
			api->GetDevice()->CreateRenderTargetView(swapChain->DxBackBufferResources[i], nullptr, rtvHandle);
			rtvHandle.Offset(1, StatRTVDescriptorOffsetSize);
		}

		return swapChain;
	}

	void D3D12SwapChain::Present(const PresentDescription& presentDesc)
	{
		UINT syncIntv = presentDesc.VSync ? 1 : 0;
		UINT flags = RHISwapChain::StatTearingSupported && !presentDesc.VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		DxSwapChain->Present(syncIntv, 0);
		CurrentBackBufferIndex = DxSwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12SwapChain::Resize(const Ptr<RenderAPI> api, Ptr<RHICommandQueue> cmdQueue, const Vector2u& newSize)
	{
		// Flush the commandqueue
		cmdQueue->Flush();
	}


	ID3D12Resource* D3D12SwapChain::GetCurrentBackBufferResource()
	{
		return DxBackBufferResources[CurrentBackBufferIndex];
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::GetCurrentRenderTargetViewHandle()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			DxRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBackBufferIndex, StatRTVDescriptorOffsetSize);
	}

	D3D12SwapChain::~D3D12SwapChain()
	{
		D3D12API::SafeRelease(DxSwapChain);
		D3D12API::SafeRelease(DxRenderTargetDescriptorHeap);

		for (int i = 0; i < StatNumBackBuffers; ++i)
		{
			D3D12API::SafeRelease(DxBackBufferResources[i]);
		}
	}
}

