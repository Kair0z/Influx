#include "InfluxGraphics/RHIDevice.h"

#include "InfluxGraphics/RHITexture.h"

namespace Influx::Graphics
{
	void RHIDevice::PostInitialize()
	{
		if (m_isInitialized && m_isCleanedUp)
		{
			return;
		}

		CreateGlobalQueues();
		CreateGlobalDescriptorHeaps();

		m_isInitialized = true;
		m_isCleanedUp = false;
	}

	void RHIDevice::PreCleanup()
	{
		if (!m_isInitialized || !m_isCleanedUp)
		{
			return;
		}

		// Todo ...

		m_isInitialized = false;
		m_isCleanedUp = true;
	}

	bool RHIDevice::GetIsDebugLayerEnabled() const
	{
		return m_isDebugLayerEnabled;
	}

	RHIDevice::TexturePtr RHIDevice::CreateTexture(const ERHIResourceState initialState, const RHITextureDesc& desc) const
	{
		TexturePtr result = new RHITexture();

		result->mp_resource = CreateTextureResource(initialState, desc.Format, desc.Dimensions, desc.NumMips);
		result->mp_renderTargetView = CreateRenderTargetView(result->mp_resource);
		result->mp_shaderResourceView = CreateShaderResourceView(result->mp_resource);

		return result;
	}

	void RHIDevice::CreateGlobalQueues()
	{
		mp_graphicsQueue = CreateCommandQueue(ERHICommandQueueType::Graphics);
		mp_computeQueue = CreateCommandQueue(ERHICommandQueueType::Compute);
	}

	void RHIDevice::CreateGlobalDescriptorHeaps()
	{
		mp_samplerDescriptorHeap	= CreateDescriptorHeap(ERHIResourceViewType::Sampler, k_maxNumSamplerDescriptorsPerHeap, true);
		mp_resourceDescriptorHeap	= CreateDescriptorHeap(ERHIResourceViewType::Resource, k_maxNumResourceDescriptorsPerHeap, true);
		mp_RTVDescriptorHeap		= CreateDescriptorHeap(ERHIResourceViewType::RTV, k_maxNumRtvDescriptorsPerHeap, false);
		mp_DSVDescriptorheap		= CreateDescriptorHeap(ERHIResourceViewType::DSV, k_maxNumDsvDescriptorsPerHeap, false);
	}

	RHICommandQueue* RHIDevice::GetGlobalGraphicsCommandQueue() const
	{
		return mp_graphicsQueue;
	}

	RHICommandQueue* RHIDevice::GetGlobalComputeCommandQueue() const
	{
		return mp_computeQueue;
	}

	RHIDevice::DescriptorHeapPtr RHIDevice::GetRTVDescriptorHeap() const
	{
		return mp_RTVDescriptorHeap;
	}

	RHIDevice::DescriptorHeapPtr RHIDevice::GetDSVDescriptorHeap() const
	{
		return mp_DSVDescriptorheap;
	}

	RHIDevice::DescriptorHeapPtr RHIDevice::GetResourceDescriptorHeap() const
	{
		return mp_resourceDescriptorHeap;
	}

	RHIDevice::DescriptorHeapPtr RHIDevice::GetSamplerDescriptorHeap() const
	{
		return mp_samplerDescriptorHeap;
	}

	RHIDevice::RenderTargetViewPtr RHIDevice::CreateRenderTargetView(const ResourcePtr viewedResource) const
	{
		return CreateRenderTargetView(GetRTVDescriptorHeap(), viewedResource);
	}

	RHIDevice::ShaderResourceViewPtr RHIDevice::CreateShaderResourceView(const ResourcePtr viewedResource) const
	{
		return CreateShaderResourceView(GetResourceDescriptorHeap(), viewedResource);
	}

	const uint64 RHIDevice::GetRTVDescriptorSize() const
	{
		return m_cachedRtvDescriptorSize;
	}

	const uint64 RHIDevice::GetDSVDescriptorSize() const
	{
		return m_cachedDsvDescriptorSize;
	}

	const uint64 RHIDevice::GetResourceDescriptorSize() const
	{
		return m_cachedResourceDescriptorSize;
	}

	const uint64 RHIDevice::GetSamplerDescriptorSize() const
	{
		return m_cachedSamplerDescriptorSize;
	}

	const uint64 RHIDevice::GetDescriptorSize(const ERHIResourceViewType type) const
	{
		switch (type)
		{
		case ERHIResourceViewType::Resource:	return GetResourceDescriptorSize();
		case ERHIResourceViewType::DSV:			return GetDSVDescriptorSize();
		case ERHIResourceViewType::RTV:			return GetRTVDescriptorSize();
		case ERHIResourceViewType::Sampler:		return GetSamplerDescriptorSize();

		default:
		case ERHIResourceViewType::Invalid:	return 0u;
		}

		return 0u;
	}
}

