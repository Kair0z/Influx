#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"
#include "InfluxGraphics/D3D12/D3D12Swapchain.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"

namespace Influx::Graphics
{
	D3D12Device::D3D12Device() : RHIDevice()
	{
		Initialize();
	}

	D3D12Device::~D3D12Device()
	{
		Cleanup();
	}

	RHICommandQueue* D3D12Device::CreateCommandQueue(const ERHICommandQueueType type) const
	{
		D3D12CommandQueue* result = new D3D12CommandQueue(type);

		result->mp_dxCommandQueue	= D3D12::CreateDxCommandQueue(GetDxDevice(), Conversion::ToDx12(type));
		result->mp_dxFence			= D3D12::CreateDxFence(GetDxDevice());

		return result;
	}

	RHISwapchain* D3D12Device::CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, RHICommandQueue* commandQueue) const
	{
		D3D12Swapchain* result = new D3D12Swapchain(dimensions.x, dimensions.y, D3D12::CheckDxgiTearingSupport());
		D3D12CommandQueue* dxCommandQueue = static_cast<D3D12CommandQueue*>(commandQueue);

		result->mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(mp_dxgiFactory, (::HWND)windowHandle, dxCommandQueue->GetDxCommandQueue(),
			dimensions.x, dimensions.y, RHISwapchain::k_numBackBuffers);

		ID3D12DescriptorHeap* newRtvDescriptorHeap 
			= result->mp_dxRenderTargetDescriptorHeap 
			= D3D12::CreateDxDescriptorHeap(GetDxDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			RHISwapchain::k_numBackBuffers, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		result->m_currentBackBufferIndex		= result->mp_dxgiSwapchain->GetCurrentBackBufferIndex();
		
		// Gather Backbuffers & RTVs
		D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(newRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		D3D12_GPU_DESCRIPTOR_HANDLE rtv_gpu_handle = newRtvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		uint64 offsetSize = GetRTVDescriptorSize();
		for (uint8 i = 0; i < RHISwapchain::k_numBackBuffers; ++i)
		{
			// Get the buffer resources
			D3D12Resource* dxBufferResource = new D3D12Resource(ERHIResourceState::Present);
			result->mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&dxBufferResource->mp_dxResource));
			result->mp_backBufferResources[i] = dxBufferResource;

			// Create the RenderTargetViews & store
			D3D12RenderTargetView* dxRenderTargetView = new D3D12RenderTargetView();
			GetDxDevice()->CreateRenderTargetView(dxBufferResource->GetDxResource(), nullptr, rtv_cpu_handle);
			dxRenderTargetView->m_dxCpuHandle = rtv_cpu_handle;
			dxRenderTargetView->m_dxGpuHandle = rtv_gpu_handle;
			result->mp_backBufferRTVs[i] = dxRenderTargetView;

			// Offset
			rtv_cpu_handle.ptr = rtv_cpu_handle.ptr + SIZE_T(offsetSize);
			rtv_gpu_handle.ptr = rtv_gpu_handle.ptr + SIZE_T(offsetSize);
		}

		return result;
	}

	RHIDescriptorHeap* D3D12Device::CreateDescriptorHeap(const ERHIDescriptorType type, uint32 numDescriptors, bool isShaderVisible) const
	{
		D3D12DescriptorHeap* result = new D3D12DescriptorHeap(type, numDescriptors, isShaderVisible);

		result->mp_dxDescriptorHeap = D3D12::CreateDxDescriptorHeap(GetDxDevice(), Conversion::ToDx12(type), numDescriptors,
			(isShaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		result->m_descriptorStride = GetDescriptorSize(type);
		result->m_occupiedSlotIndices = {};

		return result;
	}

	RHIResource* D3D12Device::CreateResource() const
	{
		return nullptr;
	}

	void D3D12Device::SetDebugLayerEnabled(bool setDebugLayerEnabled)
	{
		if (setDebugLayerEnabled)
		{
			D3D12::EnableDxDebugLayer();
		}
		else
		{
			D3D12::DisableDxDebugLayer();
		}
	}

	ID3D12Device2* D3D12Device::GetDxDevice() const
	{
		return mp_dxDevice;
	}

	IDXGIAdapter4* D3D12Device::GetDxgiAdapter() const
	{
		return mp_dxgiAdapter;
	}

	IDXGIFactory4* D3D12Device::GetDxgiFactory() const
	{
		return mp_dxgiFactory;
	}

	const uint64 D3D12Device::GetRTVDescriptorSize() const
	{
		return m_cachedRtvDescriptorSize;
	}

	const uint64 D3D12Device::GetDSVDescriptorSize() const
	{
		return m_cachedDsvDescriptorSize;
	}

	const uint64 D3D12Device::GetResourceDescriptorSize() const
	{
		return m_cachedResourceDescriptorSize;
	}

	const uint64 D3D12Device::GetSamplerDescriptorSize() const
	{
		return m_cachedSamplerDescriptorSize;
	}

	const uint64 D3D12Device::GetDescriptorSize(const ERHIDescriptorType type) const
	{
		switch (type)
		{
		case ERHIDescriptorType::Resource:	return GetResourceDescriptorSize();
		case ERHIDescriptorType::DSV:		return GetDSVDescriptorSize();
		case ERHIDescriptorType::RTV:		return GetRTVDescriptorSize();
		case ERHIDescriptorType::Sampler:	return GetSamplerDescriptorSize();

		default:
		case ERHIDescriptorType::Invalid:	return 0u;
		}

		return 0u;
	}

	void D3D12Device::Initialize()
	{
		mp_dxgiFactory	= D3D12::CreateDxgiFactory4();
		mp_dxgiAdapter	= D3D12::GetDxgiAdapter4(mp_dxgiFactory, true);
		mp_dxDevice		= D3D12::CreateDxDevice2(mp_dxgiAdapter);

		CreateGlobalDescriptorHeaps();
	}

	void D3D12Device::Cleanup()
	{

	}

	void D3D12Device::CreateGlobalDescriptorHeaps()
	{
		// Cache DescriptorSizes
		m_cachedDsvDescriptorSize			= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cachedResourceDescriptorSize		= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_cachedSamplerDescriptorSize		= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		m_cachedRtvDescriptorSize			= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Create Global Descriptor Heaps:
		mp_RTVDescriptorHeap		= static_cast<D3D12DescriptorHeap*>(this->CreateDescriptorHeap(ERHIDescriptorType::RTV, 64u, true));
		mp_DSVDescriptorheap		= static_cast<D3D12DescriptorHeap*>(this->CreateDescriptorHeap(ERHIDescriptorType::DSV, 64u, true));
		mp_samplerDescriptorHeap	= static_cast<D3D12DescriptorHeap*>(this->CreateDescriptorHeap(ERHIDescriptorType::Sampler, 16u, true));
		mp_resourceDescriptorHeap	= static_cast<D3D12DescriptorHeap*>(this->CreateDescriptorHeap(ERHIDescriptorType::Resource, 64u, true));
	}
}

