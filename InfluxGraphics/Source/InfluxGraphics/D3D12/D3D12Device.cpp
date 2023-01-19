#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"
#include "InfluxGraphics/D3D12/D3D12Swapchain.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"

#include "InfluxGraphics/D3D12/ResourceViews/D3D12RenderTargetView.h"

namespace Influx::Graphics
{
	D3D12Device::D3D12Device(bool enableDebug) : RHIDevice()
	{
		if (enableDebug)
		{
			SetDebugLayerEnabled(true);
		}

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

	RHIDevice::SwapchainPtr D3D12Device::CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, CommandQueuePtr commandQueue) const
	{
		D3D12Swapchain* result = new D3D12Swapchain(dimensions.x, dimensions.y, D3D12::CheckDxgiTearingSupport());
		D3D12CommandQueue* dxCommandQueue = static_cast<D3D12CommandQueue*>(commandQueue);

		result->m_renderTargetFormat = ERHIFormat::RGBA_8_Unorm;
		result->mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(mp_dxgiFactory, (::HWND)windowHandle, dxCommandQueue->GetDxCommandQueue(),
			dimensions.x, dimensions.y, RHISwapchain::GetNumBackBuffers(), Conversion::ToDx12(result->m_renderTargetFormat));

		result->m_currentBackBufferIndex = result->mp_dxgiSwapchain->GetCurrentBackBufferIndex();
		result->m_windowHandle = windowHandle;

		// Gather Backbuffer Resources & RTVs
		uint64 offsetSize = GetRTVDescriptorSize();
		for (uint8 i = 0; i < RHISwapchain::GetNumBackBuffers(); ++i)
		{
			// Get the buffer resources
			D3D12Resource* dxBufferResource = new D3D12Resource(ERHIResourceState::Present);
			result->mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&dxBufferResource->mp_dxResource));
			result->mp_backBufferResources[i] = dxBufferResource;

			// Create the RenderTargetViews & store
			result->mp_backBufferRTVs[i] = CreateRenderTargetView(GetRTVDescriptorHeap(), dxBufferResource);
		}

		return result;
	}

	RHIDescriptorHeap* D3D12Device::CreateDescriptorHeap(const ERHIResourceViewType type, uint32 numDescriptors, bool isShaderVisible) const
	{
		D3D12DescriptorHeap* result = new D3D12DescriptorHeap(type, numDescriptors, isShaderVisible);

		D3D12_DESCRIPTOR_HEAP_FLAGS flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		result->mp_dxDescriptorHeap = D3D12::CreateDxDescriptorHeap(GetDxDevice(), Conversion::ToDx12(type), numDescriptors, flags);

		result->m_descriptorStride = GetDescriptorSize(type);

		return result;
	}

	RHIDevice::RenderTargetViewPtr D3D12Device::CreateRenderTargetView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const
	{
		D3D12Resource* d3d12Resource = (D3D12Resource*)viewedResource;

		return CreateRenderTargetView(d3d12Resource);
	}

	D3D12RenderTargetView* D3D12Device::CreateRenderTargetView(const D3D12Resource* viewedResource) const
	{
		constexpr ERHIFormat temp_format = ERHIFormat::RGBA_8_Unorm;
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = Conversion::ToDx12(temp_format);
		
		D3D12RenderTargetView* result = new D3D12RenderTargetView(temp_format);
		if (GetRTVDescriptorHeap()->GetHandles(result->m_dxCpuHandle, result->m_dxGpuHandle) != false)
		{
			GetDxDevice()->CreateRenderTargetView(viewedResource->GetDxResource(), nullptr, result->GetDxCPUHandle());
			return result;
		}

		return nullptr;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateResource(const ERHIResourceState initialState) const
	{
		D3D12Resource* result = new D3D12Resource(initialState);

		return result;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateTextureResource(const ERHIResourceState initialState, const ERHIFormat format, const Math::Vectoru2& dimensions, const uint16 numMips) const
	{
		D3D12Resource* result = new D3D12Resource(initialState);

		using namespace D3D12::HelperStructs;
		CommittedResourceDesc textureResourceDesc =
			CommittedResourceDesc::AsTexture(Conversion::ToDx12(format), dimensions.x, dimensions.y, numMips);

		result->mp_dxResource = D3D12::CreateCommittedResource(GetDxDevice(), textureResourceDesc, Conversion::ToDx12(initialState));

		return result;
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

	D3D12DescriptorHeap* D3D12Device::GetRTVDescriptorHeap() const
	{
		return mp_RTVDescriptorHeap;
	}

	D3D12DescriptorHeap* D3D12Device::GetDSVDescriptorHeap() const
	{
		return mp_DSVDescriptorheap;
	}

	D3D12DescriptorHeap* D3D12Device::GetResourceDescriptorHeap() const
	{
		return mp_resourceDescriptorHeap;
	}

	D3D12DescriptorHeap* D3D12Device::GetSamplerDescriptorHeap() const
	{
		return mp_samplerDescriptorHeap;
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

	const uint64 D3D12Device::GetDescriptorSize(const ERHIResourceViewType type) const
	{
		switch (type)
		{
		case ERHIResourceViewType::Resource:	return GetResourceDescriptorSize();
		case ERHIResourceViewType::DSV:		return GetDSVDescriptorSize();
		case ERHIResourceViewType::RTV:		return GetRTVDescriptorSize();
		case ERHIResourceViewType::Sampler:	return GetSamplerDescriptorSize();

		default:
		case ERHIResourceViewType::Invalid:	return 0u;
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
		mp_samplerDescriptorHeap	= static_cast<D3D12DescriptorHeap*>(CreateDescriptorHeap(ERHIResourceViewType::Sampler, 16u, true));
		mp_resourceDescriptorHeap	= static_cast<D3D12DescriptorHeap*>(CreateDescriptorHeap(ERHIResourceViewType::Resource, 64u, true));
		mp_RTVDescriptorHeap		= static_cast<D3D12DescriptorHeap*>(CreateDescriptorHeap(ERHIResourceViewType::RTV, 64u, false));
		mp_DSVDescriptorheap		= static_cast<D3D12DescriptorHeap*>(CreateDescriptorHeap(ERHIResourceViewType::DSV, 64u, false));
	}
}

