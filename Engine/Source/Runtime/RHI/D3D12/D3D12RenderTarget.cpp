#include "pch.h"
#include "D3D12RenderTarget.h"

namespace Influx
{
	Ptr<D3D12RenderTarget> D3D12RenderTarget::Create(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RHIRenderTarget::ERenderTargetType type, const RenderTargetConfig& config)
	{
		// Create the Object:
		Ptr<D3D12RenderTarget> newRt = new D3D12RenderTarget(dimensions, format, type, config);

		// Create the API resources:
		auto device = api->GetDevice();
		newRt->CreateBufferResource(device);

		D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType;
		switch (newRt->mType)
		{
		default:
		case ERenderTargetType::ColourTarget: descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;

		case ERenderTargetType::DepthTarget: descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		}
		newRt->mpDescriptorHeap = D3D12API::CreateDescriptorHeap(device, descriptorHeapType, 1);

		newRt->UpdateView(device);

		return newRt;
	}

	Ptr<D3D12RenderTarget> D3D12RenderTarget::CreateFromResource(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RHIRenderTarget::ERenderTargetType type, Ptr<ID3D12Resource> bufferResource, const RenderTargetConfig& config)
	{
		// Create the Object:
		Ptr<D3D12RenderTarget> newRt = new D3D12RenderTarget(dimensions, format, type, config);
		newRt->mpBufferResource = bufferResource;

		// Create the API resources:
		auto device = api->GetDevice();

		D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType;
		switch (newRt->mType)
		{
		default:
		case ERenderTargetType::ColourTarget: descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;

		case ERenderTargetType::DepthTarget: descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		}
		newRt->mpDescriptorHeap = D3D12API::CreateDescriptorHeap(device, descriptorHeapType, 1);

		newRt->UpdateView(device);

		return newRt;
	}

	Ptr<D3D12RenderTarget> D3D12RenderTarget::CreateDepthStencil(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RenderTargetConfig& config)
	{
		return Create(api, dimensions, format, RHIRenderTarget::ERenderTargetType::DepthTarget, config);
	}

	Ptr<D3D12RenderTarget> D3D12RenderTarget::CreateRenderTarget(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RenderTargetConfig& config)
	{
		return Create(api, dimensions, format, RHIRenderTarget::ERenderTargetType::ColourTarget, config);
	}


	void D3D12RenderTarget::UpdateView(ID3D12Device2* device)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE viewHandle(mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		switch (mType)
		{
		default:
		case ERenderTargetType::ColourTarget:
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = ToDxgi(mFormat);
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			device->CreateRenderTargetView(mpBufferResource, &rtvDesc, viewHandle);
		}
			break;

		case ERenderTargetType::DepthTarget:
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = ToDxgi(mFormat);
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			device->CreateDepthStencilView(mpBufferResource, &dsvDesc, viewHandle);
		}
			break;
		}
	}

	void D3D12RenderTarget::CreateBufferResource(ID3D12Device2* device)
	{
		DXGI_FORMAT dxgiFormat = ToDxgi(mFormat);

		D3D12_CLEAR_VALUE clearValue = { dxgiFormat, {mConfig.ClearValue.x, mConfig.ClearValue.y, mConfig.ClearValue.z, mConfig.ClearValue.w} };
		D3D12_RESOURCE_STATES resourceState;
		D3D12_RESOURCE_FLAGS resourceFlags;
		switch (mType)
		{
		default:
		case ERenderTargetType::ColourTarget: 
			resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			break;

		case ERenderTargetType::DepthTarget: 
			resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			break;
		}

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
			dxgiFormat, uint64_t(mDimensions.x), uint64_t(mDimensions.y),
			mConfig.ArraySize, mConfig.MipLevels, mConfig.SampleCount, mConfig.SampleQuality, 
			resourceFlags);

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, resourceState, &clearValue,
			IID_PPV_ARGS(&mpBufferResource));
	}

	void D3D12RenderTarget::Resize(const Ptr<RenderAPI> api, const Vector2u& newSize)
	{
		// Create the API resources:
		D3D12API* dxApi = Cast<D3D12API>(api);
		auto device = dxApi->GetDevice();

		// Set new dimensions:
		mDimensions.x = newSize.x;
		mDimensions.y = newSize.y;
		
		// Recreate buffer & view:
		CreateBufferResource(device);
		UpdateView(device);
	}

	ID3D12Resource* D3D12RenderTarget::GetBufferResource() const
	{
		return mpBufferResource;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderTarget::GetViewCPUHandle() const
	{
		uint32_t descriptorIncrementSize = 0;
		uint32_t offsetInDescriptors = 0;

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			offsetInDescriptors, descriptorIncrementSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12RenderTarget::GetViewGPUHandle() const
	{
		uint32_t descriptorIncrementSize = 0;
		uint32_t offsetInDescriptors = 0;

		return CD3DX12_GPU_DESCRIPTOR_HANDLE(mpDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			offsetInDescriptors, descriptorIncrementSize);
	}

	Ptr<ID3D12DescriptorHeap> D3D12RenderTarget::GetDescriptorHeap() const
	{
		return mpDescriptorHeap;
	}

	D3D12RenderTarget::~D3D12RenderTarget()
	{
		mpDescriptorHeap->Release();
		mpBufferResource->Release();
	}
}

