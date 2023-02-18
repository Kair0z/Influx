#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"
#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "InfluxGraphics/D3D12/ResourceViews/D3D12Views.h"

namespace Influx::Graphics
{
	RHIResource::RenderTargetViewPtr D3D12Resource::CreateRenderTargetView(const DevicePtr device) const
	{
		D3D12Device* d3d12Device = (D3D12Device*)device;
		return d3d12Device->CreateRenderTargetView(this);
	}

	RHIResource::ShaderResourceViewPtr D3D12Resource::CreateShaderResourceView(const DevicePtr device) const
	{
		D3D12Device* d3d12Device = (D3D12Device*)device;
		return d3d12Device->CreateShaderResourceView(this);
	}

	void D3D12Resource::OnTransitionState(const ERHIResourceState before, const ERHIResourceState after)
	{

	}

	ID3D12Resource* D3D12Resource::GetDxResource() const
	{
		return mp_dxResource;
	}

	D3D12Resource::~D3D12Resource()
	{
		D3D12::SafeRelease(mp_dxResource);
	}
}